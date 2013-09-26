//===- lib/ReaderWriter/ELF/X86_64/X86_64LinkingContext.cpp ---------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Atoms.h"
#include "X86_64LinkingContext.h"

#include "lld/Core/File.h"
#include "lld/Core/Instrumentation.h"
#include "lld/Core/Parallel.h"
#include "lld/Core/Pass.h"
#include "lld/Core/PassManager.h"
#include "lld/ReaderWriter/Simple.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringSwitch.h"

using namespace lld;
using namespace lld::elf;

namespace {
using namespace llvm::ELF;

// .got values
const uint8_t x86_64GotAtomContent[8] = { 0 };
const uint8_t x86_64InitFiniAtomContent[8] = { 0 };

// .plt value (entry 0)
const uint8_t x86_64Plt0AtomContent[16] = {
  0xff, 0x35, 0x00, 0x00, 0x00, 0x00, // pushq GOT+8(%rip)
  0xff, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp *GOT+16(%rip)
  0x90, 0x90, 0x90, 0x90              // nopnopnop
};

// .plt values (other entries)
const uint8_t x86_64PltAtomContent[16] = {
  0xff, 0x25, 0x00, 0x00, 0x00, 0x00, // jmpq *gotatom(%rip)
  0x68, 0x00, 0x00, 0x00, 0x00,       // pushq reloc-index
  0xe9, 0x00, 0x00, 0x00, 0x00        // jmpq plt[-1]
};

/// \brief Atoms that are used by X86_64 dynamic linking
class X86_64GOTAtom : public GOTAtom {
public:
  X86_64GOTAtom(const File &f, StringRef secName) : GOTAtom(f, secName) {}

  virtual ArrayRef<uint8_t> rawContent() const {
    return ArrayRef<uint8_t>(x86_64GotAtomContent, 8);
  }
};

class X86_64PLT0Atom : public PLT0Atom {
public:
  X86_64PLT0Atom(const File &f) : PLT0Atom(f) {
#ifndef NDEBUG
    _name = ".PLT0";
#endif
  }
  virtual ArrayRef<uint8_t> rawContent() const {
    return ArrayRef<uint8_t>(x86_64Plt0AtomContent, 16);
  }
};

class X86_64PLTAtom : public PLTAtom {
public:
  X86_64PLTAtom(const File &f, StringRef secName) : PLTAtom(f, secName) {}

  virtual ArrayRef<uint8_t> rawContent() const {
    return ArrayRef<uint8_t>(x86_64PltAtomContent, 16);
  }
};

class ELFPassFile : public SimpleFile {
public:
  ELFPassFile(const ELFLinkingContext &eti) : SimpleFile(eti, "ELFPassFile") {}

  llvm::BumpPtrAllocator _alloc;
};

/// \brief Create GOT and PLT entries for relocations. Handles standard GOT/PLT
/// along with IFUNC and TLS.
template <class Derived> class GOTPLTPass : public Pass {
  /// \brief Handle a specific reference.
  void handleReference(const DefinedAtom &atom, const Reference &ref) {
    switch (ref.kind()) {
    case R_X86_64_32S:
      static_cast<Derived *>(this)->handle32S(ref);
      break;
    case R_X86_64_PLT32:
      static_cast<Derived *>(this)->handlePLT32(ref);
      break;
    case R_X86_64_PC32:
      static_cast<Derived *>(this)->handlePC32(ref);
      break;
    case R_X86_64_GOTTPOFF: // GOT Thread Pointer Offset
      static_cast<Derived *>(this)->handleGOTTPOFF(ref);
      break;
    case R_X86_64_GOTPCREL:
      static_cast<Derived *>(this)->handleGOTPCREL(ref);
      break;
    }
  }

protected:
  /// \brief get the PLT entry for a given IFUNC Atom.
  ///
  /// If the entry does not exist. Both the GOT and PLT entry is created.
  const PLTAtom *getIFUNCPLTEntry(const DefinedAtom *da) {
    auto plt = _pltMap.find(da);
    if (plt != _pltMap.end())
      return plt->second;
    auto ga = new (_file._alloc) X86_64GOTAtom(_file, ".got.plt");
    ga->addReference(R_X86_64_IRELATIVE, 0, da, 0);
    auto pa = new (_file._alloc) X86_64PLTAtom(_file, ".plt");
    pa->addReference(R_X86_64_PC32, 2, ga, -4);
#ifndef NDEBUG
    ga->_name = "__got_ifunc_";
    ga->_name += da->name();
    pa->_name = "__plt_ifunc_";
    pa->_name += da->name();
#endif
    _gotMap[da] = ga;
    _pltMap[da] = pa;
    _gotVector.push_back(ga);
    _pltVector.push_back(pa);
    return pa;
  }

  /// \brief Redirect the call to the PLT stub for the target IFUNC.
  ///
  /// This create a PLT and GOT entry for the IFUNC if one does not exist. The
  /// GOT entry and a IRELATIVE relocation to the original target resolver.
  ErrorOr<void> handleIFUNC(const Reference &ref) {
    auto target = dyn_cast_or_null<const DefinedAtom>(ref.target());
    if (target && target->contentType() == DefinedAtom::typeResolver)
      const_cast<Reference &>(ref).setTarget(getIFUNCPLTEntry(target));
    return error_code::success();
  }

  /// \brief Create a GOT entry for the TP offset of a TLS atom.
  const GOTAtom *getGOTTPOFF(const Atom *atom) {
    auto got = _gotMap.find(atom);
    if (got == _gotMap.end()) {
      auto g = new (_file._alloc) X86_64GOTAtom(_file, ".got");
      g->addReference(R_X86_64_TPOFF64, 0, atom, 0);
#ifndef NDEBUG
      g->_name = "__got_tls_";
      g->_name += atom->name();
#endif
      _gotMap[atom] = g;
      _gotVector.push_back(g);
      return g;
    }
    return got->second;
  }

  /// \brief Create a TPOFF64 GOT entry and change the relocation to a PC32 to
  /// the GOT.
  void handleGOTTPOFF(const Reference &ref) {
    const_cast<Reference &>(ref).setTarget(getGOTTPOFF(ref.target()));
    const_cast<Reference &>(ref).setKind(R_X86_64_PC32);
  }

  /// \brief Create a GOT entry containing 0.
  const GOTAtom *getNullGOT() {
    if (!_null) {
      _null = new (_file._alloc) X86_64GOTAtom(_file, ".got.plt");
#ifndef NDEBUG
      _null->_name = "__got_null";
#endif
    }
    return _null;
  }

  const GOTAtom *getGOT(const DefinedAtom *da) {
    auto got = _gotMap.find(da);
    if (got == _gotMap.end()) {
      auto g = new (_file._alloc) X86_64GOTAtom(_file, ".got");
      g->addReference(R_X86_64_64, 0, da, 0);
#ifndef NDEBUG
      g->_name = "__got_";
      g->_name += da->name();
#endif
      _gotMap[da] = g;
      _gotVector.push_back(g);
      return g;
    }
    return got->second;
  }

  /// \brief Handle a GOTPCREL relocation to an undefined weak atom by using a
  /// null GOT entry.
  void handleGOTPCREL(const Reference &ref) {
    const_cast<Reference &>(ref).setKind(R_X86_64_PC32);
    if (isa<UndefinedAtom>(ref.target()))
      const_cast<Reference &>(ref).setTarget(getNullGOT());
    else if (const DefinedAtom *da = dyn_cast<const DefinedAtom>(ref.target()))
      const_cast<Reference &>(ref).setTarget(getGOT(da));
  }

public:
  GOTPLTPass(const ELFLinkingContext &ctx)
      : _file(ctx), _null(nullptr), _PLT0(nullptr), _got0(nullptr),
        _got1(nullptr) {}

  /// \brief Do the pass.
  ///
  /// The goal here is to first process each reference individually. Each call
  /// to handleReference may modify the reference itself and/or create new
  /// atoms which must be stored in one of the maps below.
  ///
  /// After all references are handled, the atoms created during that are all
  /// added to mf.
  virtual void perform(MutableFile &mf) {
    ScopedTask task(getDefaultDomain(), "X86-64 GOT/PLT Pass");
    // Process all references.
    for (const auto &atom : mf.defined())
      for (const auto &ref : *atom)
        handleReference(*atom, *ref);

    // Add all created atoms to the link.
    uint64_t ordinal = 0;
    if (_PLT0) {
      _PLT0->setOrdinal(ordinal++);
      mf.addAtom(*_PLT0);
    }
    for (auto &plt : _pltVector) {
      plt->setOrdinal(ordinal++);
      mf.addAtom(*plt);
    }
    if (_null) {
      _null->setOrdinal(ordinal++);
      mf.addAtom(*_null);
    }
    if (_PLT0) {
      _got0->setOrdinal(ordinal++);
      _got1->setOrdinal(ordinal++);
      mf.addAtom(*_got0);
      mf.addAtom(*_got1);
    }
    for (auto &got : _gotVector) {
      got->setOrdinal(ordinal++);
      mf.addAtom(*got);
    }
    for (auto obj : _objectVector) {
      obj->setOrdinal(ordinal++);
      mf.addAtom(*obj);
    }
  }

protected:
  /// \brief Owner of all the Atoms created by this pass.
  ELFPassFile _file;

  /// \brief Map Atoms to their GOT entries.
  llvm::DenseMap<const Atom *, GOTAtom *> _gotMap;

  /// \brief Map Atoms to their PLT entries.
  llvm::DenseMap<const Atom *, PLTAtom *> _pltMap;

  /// \brief Map Atoms to their Object entries.
  llvm::DenseMap<const Atom *, ObjectAtom *> _objectMap;

  /// \brief the list of GOT/PLT atoms
  std::vector<GOTAtom *> _gotVector;
  std::vector<PLTAtom *> _pltVector;
  std::vector<ObjectAtom *> _objectVector;

  /// \brief GOT entry that is always 0. Used for undefined weaks.
  GOTAtom *_null;

  /// \brief The got and plt entries for .PLT0. This is used to call into the
  /// dynamic linker for symbol resolution.
  /// @{
  PLT0Atom *_PLT0;
  GOTAtom *_got0;
  GOTAtom *_got1;
  /// @}
};

/// This implements the static relocation model. Meaning GOT and PLT entries are
/// not created for references that can be directly resolved. These are
/// converted to a direct relocation. For entries that do require a GOT or PLT
/// entry, that entry is statically bound.
///
/// TLS always assumes module 1 and attempts to remove indirection.
class StaticGOTPLTPass LLVM_FINAL : public GOTPLTPass<StaticGOTPLTPass> {
public:
  StaticGOTPLTPass(const elf::X86_64LinkingContext &ctx) : GOTPLTPass(ctx) {}

  ErrorOr<void> handlePLT32(const Reference &ref) {
    // __tls_get_addr is handled elsewhere.
    if (ref.target() && ref.target()->name() == "__tls_get_addr") {
      const_cast<Reference &>(ref).setKind(R_X86_64_NONE);
      return error_code::success();
    } else
      // Static code doesn't need PLTs.
      const_cast<Reference &>(ref).setKind(R_X86_64_PC32);
    // Handle IFUNC.
    if (const DefinedAtom *da =
            dyn_cast_or_null<const DefinedAtom>(ref.target()))
      if (da->contentType() == DefinedAtom::typeResolver)
        return handleIFUNC(ref);
    return error_code::success();
  }

  ErrorOr<void> handlePC32(const Reference &ref) { return handleIFUNC(ref); }

  ErrorOr<void> handle32S(const Reference &ref) {
    // Do nothing.
    return error_code::success();
  }
};

class DynamicGOTPLTPass LLVM_FINAL : public GOTPLTPass<DynamicGOTPLTPass> {
public:
  DynamicGOTPLTPass(const elf::X86_64LinkingContext &ctx) : GOTPLTPass(ctx) {}

  const PLT0Atom *getPLT0() {
    if (_PLT0)
      return _PLT0;
    // Fill in the null entry.
    getNullGOT();
    _PLT0 = new (_file._alloc) X86_64PLT0Atom(_file);
    _got0 = new (_file._alloc) X86_64GOTAtom(_file, ".got.plt");
    _got1 = new (_file._alloc) X86_64GOTAtom(_file, ".got.plt");
    _PLT0->addReference(R_X86_64_PC32, 2, _got0, -4);
    _PLT0->addReference(R_X86_64_PC32, 8, _got1, -4);
#ifndef NDEBUG
    _got0->_name = "__got0";
    _got1->_name = "__got1";
#endif
    return _PLT0;
  }

  const PLTAtom *getPLTEntry(const Atom *a) {
    auto plt = _pltMap.find(a);
    if (plt != _pltMap.end())
      return plt->second;
    auto ga = new (_file._alloc) X86_64GOTAtom(_file, ".got.plt");
    ga->addReference(R_X86_64_JUMP_SLOT, 0, a, 0);
    auto pa = new (_file._alloc) X86_64PLTAtom(_file, ".plt");
    pa->addReference(R_X86_64_PC32, 2, ga, -4);
    pa->addReference(LLD_R_X86_64_GOTRELINDEX, 7, ga, 0);
    pa->addReference(R_X86_64_PC32, 12, getPLT0(), -4);
    // Set the starting address of the got entry to the second instruction in
    // the plt entry.
    ga->addReference(R_X86_64_64, 0, pa, 6);
#ifndef NDEBUG
    ga->_name = "__got_";
    ga->_name += a->name();
    pa->_name = "__plt_";
    pa->_name += a->name();
#endif
    _gotMap[a] = ga;
    _pltMap[a] = pa;
    _gotVector.push_back(ga);
    _pltVector.push_back(pa);
    return pa;
  }

  const ObjectAtom *getObjectEntry(const SharedLibraryAtom *a) {
    auto obj = _objectMap.find(a);
    if (obj != _objectMap.end())
      return obj->second;

    auto oa = new (_file._alloc) ObjectAtom(_file);
    oa->addReference(R_X86_64_COPY, 0, a, 0);

    oa->_name = a->name();
    oa->_size = a->size();

    _objectMap[a] = oa;
    _objectVector.push_back(oa);
    return oa;
  }

  ErrorOr<void> handle32S(const Reference &ref) {
    if (auto sla = dyn_cast_or_null<SharedLibraryAtom>(ref.target()))
      if (sla->type() == SharedLibraryAtom::Type::Data)
        const_cast<Reference &>(ref).setTarget(getObjectEntry(sla));
    return error_code::success();
  }

  ErrorOr<void> handlePLT32(const Reference &ref) {
    // Turn this into a PC32 to the PLT entry.
    const_cast<Reference &>(ref).setKind(R_X86_64_PC32);
    // Handle IFUNC.
    if (const DefinedAtom *da =
            dyn_cast_or_null<const DefinedAtom>(ref.target()))
      if (da->contentType() == DefinedAtom::typeResolver)
        return handleIFUNC(ref);
    if (isa<const SharedLibraryAtom>(ref.target()))
      const_cast<Reference &>(ref).setTarget(getPLTEntry(ref.target()));
    return error_code::success();
  }

  ErrorOr<void> handlePC32(const Reference &ref) {
    if (!ref.target())
      return error_code::success();
    if (auto sla = dyn_cast<SharedLibraryAtom>(ref.target())) {
      if (sla->type() == SharedLibraryAtom::Type::Code)
        return handlePLT32(ref);
      else
        return handleGOTPCREL(ref);
    }
    return handleIFUNC(ref);
  }

  const GOTAtom *getSharedGOT(const SharedLibraryAtom *sla) {
    auto got = _gotMap.find(sla);
    if (got == _gotMap.end()) {
      auto g = new (_file._alloc) X86_64GOTAtom(_file, ".got.dyn");
      g->addReference(R_X86_64_GLOB_DAT, 0, sla, 0);
#ifndef NDEBUG
      g->_name = "__got_";
      g->_name += sla->name();
#endif
      _gotMap[sla] = g;
      _gotVector.push_back(g);
      return g;
    }
    return got->second;
  }

  ErrorOr<void> handleGOTPCREL(const Reference &ref) {
    const_cast<Reference &>(ref).setKind(R_X86_64_PC32);
    if (isa<UndefinedAtom>(ref.target()))
      const_cast<Reference &>(ref).setTarget(getNullGOT());
    else if (const DefinedAtom *da = dyn_cast<const DefinedAtom>(ref.target()))
      const_cast<Reference &>(ref).setTarget(getGOT(da));
    else if (const auto sla = dyn_cast<const SharedLibraryAtom>(ref.target()))
      const_cast<Reference &>(ref).setTarget(getSharedGOT(sla));
    return error_code::success();
  }
};

// X86_64InitFini Atom
class X86_64InitAtom : public InitFiniAtom {
public:
  X86_64InitAtom(const File &f, StringRef function)
      : InitFiniAtom(f, ".init_array") {
#ifndef NDEBUG
    _name = "__init_fn_";
    _name += function;
#endif
  }
  virtual ArrayRef<uint8_t> rawContent() const {
    return ArrayRef<uint8_t>(x86_64InitFiniAtomContent, 8);
  }
  virtual Alignment alignment() const { return Alignment(3); }
};

class X86_64FiniAtom : public InitFiniAtom {
public:
  X86_64FiniAtom(const File &f, StringRef function)
      : InitFiniAtom(f, ".fini_array") {
#ifndef NDEBUG
    _name = "__fini_fn_";
    _name += function;
#endif
  }
  virtual ArrayRef<uint8_t> rawContent() const {
    return ArrayRef<uint8_t>(x86_64InitFiniAtomContent, 8);
  }

  virtual Alignment alignment() const { return Alignment(3); }
};

class X86_64InitFiniFile : public SimpleFile {
public:
  X86_64InitFiniFile(const ELFLinkingContext &context):
    SimpleFile(context, "command line option -init/-fini")
  {}

  void addInitFunction(StringRef name) {
    Atom *initFunctionAtom = new (_allocator) SimpleUndefinedAtom(*this, name);
    X86_64InitAtom *initAtom =
           (new (_allocator) X86_64InitAtom(*this, name));
    initAtom->addReference(llvm::ELF::R_X86_64_64, 0, initFunctionAtom, 0);
    initAtom->setOrdinal(_ordinal++);
    addAtom(*initFunctionAtom);
    addAtom(*initAtom);
  }

  void addFiniFunction(StringRef name) {
    Atom *finiFunctionAtom = new (_allocator) SimpleUndefinedAtom(*this, name);
    X86_64FiniAtom *finiAtom =
           (new (_allocator) X86_64FiniAtom(*this, name));
    finiAtom->addReference(llvm::ELF::R_X86_64_64, 0, finiFunctionAtom, 0);
    finiAtom->setOrdinal(_ordinal++);
    addAtom(*finiFunctionAtom);
    addAtom(*finiAtom);
  }

private:
  llvm::BumpPtrAllocator _allocator;
};

} // end anon namespace


void elf::X86_64LinkingContext::addPasses(PassManager &pm) const {
  switch (_outputELFType) {
  case llvm::ELF::ET_EXEC:
    if (_isStaticExecutable)
      pm.add(std::unique_ptr<Pass>(new StaticGOTPLTPass(*this)));
    else
      pm.add(std::unique_ptr<Pass>(new DynamicGOTPLTPass(*this)));
    break;
  case llvm::ELF::ET_DYN:
    pm.add(std::unique_ptr<Pass>(new DynamicGOTPLTPass(*this)));
    break;
  case llvm::ELF::ET_REL:
    break;
  default:
    llvm_unreachable("Unhandled output file type");
  }
  ELFLinkingContext::addPasses(pm);
}

std::vector<std::unique_ptr<File>>
  elf::X86_64LinkingContext::createInternalFiles(){
  std::vector<std::unique_ptr<File> > result =
    ELFLinkingContext::createInternalFiles();
  std::unique_ptr<X86_64InitFiniFile>
    initFiniFile(new X86_64InitFiniFile(*this));
  for (auto ai:initFunctions())
    initFiniFile->addInitFunction(ai);
  for (auto ai:finiFunctions())
    initFiniFile->addFiniFunction(ai);
  result.push_back(std::move(initFiniFile));
  return result;
}

#define LLD_CASE(name) .Case(#name, llvm::ELF::name)

ErrorOr<Reference::Kind>
elf::X86_64LinkingContext::relocKindFromString(StringRef str) const {
  int32_t ret = llvm::StringSwitch<int32_t>(str) LLD_CASE(R_X86_64_NONE)
      LLD_CASE(R_X86_64_64) LLD_CASE(R_X86_64_PC32) LLD_CASE(R_X86_64_GOT32)
      LLD_CASE(R_X86_64_PLT32) LLD_CASE(R_X86_64_COPY)
      LLD_CASE(R_X86_64_GLOB_DAT) LLD_CASE(R_X86_64_JUMP_SLOT)
      LLD_CASE(R_X86_64_RELATIVE) LLD_CASE(R_X86_64_GOTPCREL)
      LLD_CASE(R_X86_64_32) LLD_CASE(R_X86_64_32S) LLD_CASE(R_X86_64_16)
      LLD_CASE(R_X86_64_PC16) LLD_CASE(R_X86_64_8) LLD_CASE(R_X86_64_PC8)
      LLD_CASE(R_X86_64_DTPMOD64) LLD_CASE(R_X86_64_DTPOFF64)
      LLD_CASE(R_X86_64_TPOFF64) LLD_CASE(R_X86_64_TLSGD)
      LLD_CASE(R_X86_64_TLSLD) LLD_CASE(R_X86_64_DTPOFF32)
      LLD_CASE(R_X86_64_GOTTPOFF) LLD_CASE(R_X86_64_TPOFF32)
      LLD_CASE(R_X86_64_PC64) LLD_CASE(R_X86_64_GOTOFF64)
      LLD_CASE(R_X86_64_GOTPC32) LLD_CASE(R_X86_64_GOT64)
      LLD_CASE(R_X86_64_GOTPCREL64) LLD_CASE(R_X86_64_GOTPC64)
      LLD_CASE(R_X86_64_GOTPLT64) LLD_CASE(R_X86_64_PLTOFF64)
      LLD_CASE(R_X86_64_SIZE32) LLD_CASE(R_X86_64_SIZE64)
      LLD_CASE(R_X86_64_GOTPC32_TLSDESC) LLD_CASE(R_X86_64_TLSDESC_CALL)
      LLD_CASE(R_X86_64_TLSDESC) LLD_CASE(R_X86_64_IRELATIVE)
          .Case("LLD_R_X86_64_GOTRELINDEX", LLD_R_X86_64_GOTRELINDEX)
          .Default(-1);

  if (ret == -1)
    return make_error_code(yaml_reader_error::illegal_value);
  return ret;
}

#undef LLD_CASE

#define LLD_CASE(name)                                                         \
  case llvm::ELF::name:                                                        \
  return std::string(#name);

ErrorOr<std::string>
elf::X86_64LinkingContext::stringFromRelocKind(Reference::Kind kind) const {
  switch (kind) {
    LLD_CASE(R_X86_64_NONE)
    LLD_CASE(R_X86_64_64)
    LLD_CASE(R_X86_64_PC32)
    LLD_CASE(R_X86_64_GOT32)
    LLD_CASE(R_X86_64_PLT32)
    LLD_CASE(R_X86_64_COPY)
    LLD_CASE(R_X86_64_GLOB_DAT)
    LLD_CASE(R_X86_64_JUMP_SLOT)
    LLD_CASE(R_X86_64_RELATIVE)
    LLD_CASE(R_X86_64_GOTPCREL)
    LLD_CASE(R_X86_64_32)
    LLD_CASE(R_X86_64_32S)
    LLD_CASE(R_X86_64_16)
    LLD_CASE(R_X86_64_PC16)
    LLD_CASE(R_X86_64_8)
    LLD_CASE(R_X86_64_PC8)
    LLD_CASE(R_X86_64_DTPMOD64)
    LLD_CASE(R_X86_64_DTPOFF64)
    LLD_CASE(R_X86_64_TPOFF64)
    LLD_CASE(R_X86_64_TLSGD)
    LLD_CASE(R_X86_64_TLSLD)
    LLD_CASE(R_X86_64_DTPOFF32)
    LLD_CASE(R_X86_64_GOTTPOFF)
    LLD_CASE(R_X86_64_TPOFF32)
    LLD_CASE(R_X86_64_PC64)
    LLD_CASE(R_X86_64_GOTOFF64)
    LLD_CASE(R_X86_64_GOTPC32)
    LLD_CASE(R_X86_64_GOT64)
    LLD_CASE(R_X86_64_GOTPCREL64)
    LLD_CASE(R_X86_64_GOTPC64)
    LLD_CASE(R_X86_64_GOTPLT64)
    LLD_CASE(R_X86_64_PLTOFF64)
    LLD_CASE(R_X86_64_SIZE32)
    LLD_CASE(R_X86_64_SIZE64)
    LLD_CASE(R_X86_64_GOTPC32_TLSDESC)
    LLD_CASE(R_X86_64_TLSDESC_CALL)
    LLD_CASE(R_X86_64_TLSDESC)
    LLD_CASE(R_X86_64_IRELATIVE)
  case LLD_R_X86_64_GOTRELINDEX:
    return std::string("LLD_R_X86_64_GOTRELINDEX");
  }

  return make_error_code(yaml_reader_error::illegal_value);
}

