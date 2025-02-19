//===--- Bridging/DeclAttributeBridging.cpp--------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2022-2024 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "swift/AST/ASTBridging.h"

#include "swift/AST/ASTContext.h"
#include "swift/AST/Attr.h"
#include "swift/AST/Expr.h"
#include "swift/AST/Identifier.h"
#include "swift/Basic/Assertions.h"

using namespace swift;

//===----------------------------------------------------------------------===//
// MARK: DeclAttributes
//===----------------------------------------------------------------------===//

// Define `.asDeclAttribute` on each BridgedXXXAttr type.
#define SIMPLE_DECL_ATTR(...)
#define DECL_ATTR(_, CLASS, ...)                                               \
  BridgedDeclAttribute Bridged##CLASS##Attr_asDeclAttribute(                   \
      Bridged##CLASS##Attr attr) {                                             \
    return static_cast<DeclAttribute *>(attr.unbridged());                     \
  }
#include "swift/AST/DeclAttr.def"

BridgedDeclAttrKind BridgedDeclAttrKind_fromString(BridgedStringRef cStr) {
  auto optKind = DeclAttribute::getAttrKindFromString(cStr.unbridged());
  if (!optKind)
    return BridgedDeclAttrKindNone;
  switch (*optKind) {
#define DECL_ATTR(_, CLASS, ...)                                               \
  case DeclAttrKind::CLASS:                                                    \
    return BridgedDeclAttrKind##CLASS;
#include "swift/AST/DeclAttr.def"
  }
}

std::optional<DeclAttrKind> unbridged(BridgedDeclAttrKind kind) {
  switch (kind) {
#define DECL_ATTR(_, CLASS, ...)                                               \
  case BridgedDeclAttrKind##CLASS:                                             \
    return DeclAttrKind::CLASS;
#include "swift/AST/DeclAttr.def"
  case BridgedDeclAttrKindNone:
    return std::nullopt;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedDeclAttribute BridgedDeclAttribute_createSimple(
    BridgedASTContext cContext, BridgedDeclAttrKind cKind,
    BridgedSourceLoc cAtLoc, BridgedSourceLoc cAttrLoc) {
  auto optKind = unbridged(cKind);
  assert(optKind && "creating attribute of invalid kind?");
  return DeclAttribute::createSimple(cContext.unbridged(), *optKind,
                                     cAtLoc.unbridged(), cAttrLoc.unbridged());
}

bool BridgedDeclAttribute_isDeclModifier(BridgedDeclAttrKind cKind) {
  auto optKind = unbridged(cKind);
  if (!optKind)
    return false;
  return DeclAttribute::isDeclModifier(*optKind);
}

void BridgedDeclAttributes_add(BridgedDeclAttributes *cAttrs,
                               BridgedDeclAttribute cAdd) {
  auto attrs = cAttrs->unbridged();
  attrs.add(cAdd.unbridged());
  *cAttrs = attrs;
}

static AvailableAttr::Kind unbridge(BridgedAvailableAttrKind value) {
  switch (value) {
  case BridgedAvailableAttrKindDefault:
    return AvailableAttr::Kind::Default;
  case BridgedAvailableAttrKindDeprecated:
    return AvailableAttr::Kind::Deprecated;
  case BridgedAvailableAttrKindUnavailable:
    return AvailableAttr::Kind::Unavailable;
  case BridgedAvailableAttrKindNoAsync:
    return AvailableAttr::Kind::NoAsync;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedAvailableAttr BridgedAvailableAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedAvailabilityDomain cDomain,
    BridgedSourceLoc cDomainLoc, BridgedAvailableAttrKind cKind,
    BridgedStringRef cMessage, BridgedStringRef cRenamed,
    BridgedVersionTuple cIntroduced, BridgedSourceRange cIntroducedRange,
    BridgedVersionTuple cDeprecated, BridgedSourceRange cDeprecatedRange,
    BridgedVersionTuple cObsoleted, BridgedSourceRange cObsoletedRange) {
  return new (cContext.unbridged())
      AvailableAttr(cAtLoc.unbridged(), cRange.unbridged(), cDomain.unbridged(),
                    cDomainLoc.unbridged(), unbridge(cKind),
                    cMessage.unbridged(), cRenamed.unbridged(),
                    cIntroduced.unbridged(), cIntroducedRange.unbridged(),
                    cDeprecated.unbridged(), cDeprecatedRange.unbridged(),
                    cObsoleted.unbridged(), cObsoletedRange.unbridged(),
                    /*Implicit=*/false,
                    /*IsSPI=*/false);
}

BridgedAvailableAttr BridgedAvailableAttr_createParsedIdentifier(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedIdentifier cDomainIdentifier,
    BridgedSourceLoc cDomainLoc, BridgedAvailableAttrKind cKind,
    BridgedStringRef cMessage, BridgedStringRef cRenamed,
    BridgedVersionTuple cIntroduced, BridgedSourceRange cIntroducedRange,
    BridgedVersionTuple cDeprecated, BridgedSourceRange cDeprecatedRange,
    BridgedVersionTuple cObsoleted, BridgedSourceRange cObsoletedRange) {

  return new (cContext.unbridged())
      AvailableAttr(cAtLoc.unbridged(), cRange.unbridged(),
                    cDomainIdentifier.unbridged(), cDomainLoc.unbridged(),
                    unbridge(cKind), cMessage.unbridged(), cRenamed.unbridged(),
                    cIntroduced.unbridged(), cIntroducedRange.unbridged(),
                    cDeprecated.unbridged(), cDeprecatedRange.unbridged(),
                    cObsoleted.unbridged(), cObsoletedRange.unbridged(),
                    /*Implicit=*/false,
                    /*IsSPI=*/false);
}

static std::optional<AccessLevel> unbridge(BridgedAccessLevel level) {
  switch (level) {
  case BridgedAccessLevelPrivate:
    return AccessLevel::Private;
  case BridgedAccessLevelFilePrivate:
    return AccessLevel::FilePrivate;
  case BridgedAccessLevelInternal:
    return AccessLevel::Internal;
  case BridgedAccessLevelPackage:
    return AccessLevel::Package;
  case BridgedAccessLevelPublic:
    return AccessLevel::Public;
  case BridgedAccessLevelOpen:
    return AccessLevel::Open;
  case BridgedAccessLevelNone:
    return std::nullopt;
  }
  llvm_unreachable("unhandled BridgedAccessLevel");
}

BridgedABIAttr BridgedABIAttr_createParsed(BridgedASTContext cContext,
                                           BridgedSourceLoc atLoc,
                                           BridgedSourceRange range,
                                           BridgedNullableDecl abiDecl) {
  return new (cContext.unbridged()) ABIAttr(abiDecl.unbridged(),
                                            atLoc.unbridged(),
                                            range.unbridged(),
                                            /*isImplicit=*/false);
}

BridgedAccessControlAttr
BridgedAccessControlAttr_createParsed(BridgedASTContext cContext,
                                      BridgedSourceRange cRange,
                                      BridgedAccessLevel cAccessLevel) {
  return new (cContext.unbridged()) AccessControlAttr(
      /*atLoc=*/{}, cRange.unbridged(), unbridge(cAccessLevel).value());
}

BridgedAlignmentAttr
BridgedAlignmentAttr_createParsed(BridgedASTContext cContext,
                                  BridgedSourceLoc cAtLoc,
                                  BridgedSourceRange cRange, size_t cValue) {
  return new (cContext.unbridged()) AlignmentAttr(
      cValue, cAtLoc.unbridged(), cRange.unbridged(), /*Implicit=*/false);
}

BridgedAllowFeatureSuppressionAttr
BridgedAllowFeatureSuppressionAttr_createParsed(BridgedASTContext cContext,
                                                BridgedSourceLoc cAtLoc,
                                                BridgedSourceRange cRange,
                                                bool inverted,
                                                BridgedArrayRef cFeatures) {
  SmallVector<Identifier> features;
  for (auto elem : cFeatures.unbridged<BridgedIdentifier>())
    features.push_back(elem.unbridged());
  return AllowFeatureSuppressionAttr::create(
      cContext.unbridged(), cAtLoc.unbridged(), cRange.unbridged(),
      /*implicit*/ false, inverted, features);
}

BridgedBackDeployedAttr BridgedBackDeployedAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedPlatformKind cPlatform,
    BridgedVersionTuple cVersion) {
  return new (cContext.unbridged()) BackDeployedAttr(
      cAtLoc.unbridged(), cRange.unbridged(), unbridge(cPlatform),
      cVersion.unbridged(), /*Implicit=*/false);
}

BridgedCDeclAttr BridgedCDeclAttr_createParsed(BridgedASTContext cContext,
                                               BridgedSourceLoc cAtLoc,
                                               BridgedSourceRange cRange,
                                               BridgedStringRef cName) {
  return new (cContext.unbridged())
      CDeclAttr(cName.unbridged(), cAtLoc.unbridged(), cRange.unbridged(),
                /*Implicit=*/false);
}

BridgedCustomAttr BridgedCustomAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc, BridgedTypeRepr cType,
    BridgedNullableCustomAttributeInitializer cInitContext,
    BridgedNullableArgumentList cArgumentList) {
  ASTContext &context = cContext.unbridged();
  return CustomAttr::create(
      context, cAtLoc.unbridged(), new (context) TypeExpr(cType.unbridged()),
      cInitContext.unbridged(), cArgumentList.unbridged());
}

BridgedDynamicReplacementAttr BridgedDynamicReplacementAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceLoc cAttrNameLoc, BridgedSourceLoc cLParenLoc,
    BridgedDeclNameRef cReplacedFunction, BridgedSourceLoc cRParenLoc) {
  return DynamicReplacementAttr::create(
      cContext.unbridged(), cAtLoc.unbridged(), cAttrNameLoc.unbridged(),
      cLParenLoc.unbridged(), cReplacedFunction.unbridged(),
      cRParenLoc.unbridged());
}

BridgedDocumentationAttr BridgedDocumentationAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedStringRef cMetadata,
    BridgedAccessLevel cAccessLevel) {
  return new (cContext.unbridged()) DocumentationAttr(
      cAtLoc.unbridged(), cRange.unbridged(), cMetadata.unbridged(),
      unbridge(cAccessLevel), /*implicit=*/false);
}

static EffectsKind unbridged(BridgedEffectsKind kind) {
  switch (kind) {
  case BridgedEffectsKindReadNone:
    return EffectsKind::ReadNone;
  case BridgedEffectsKindReadOnly:
    return EffectsKind::ReadOnly;
  case BridgedEffectsKindReleaseNone:
    return EffectsKind::ReleaseNone;
  case BridgedEffectsKindReadWrite:
    return EffectsKind::ReadWrite;
  case BridgedEffectsKindUnspecified:
    return EffectsKind::Unspecified;
  case BridgedEffectsKindCustom:
    return EffectsKind::Custom;
  }
  llvm_unreachable("unhandled kind");
}

BridgedEffectsAttr BridgedEffectsAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedEffectsKind cEffectKind) {
  return new (cContext.unbridged()) EffectsAttr(
      cAtLoc.unbridged(), cRange.unbridged(), unbridged(cEffectKind));
}

BridgedEffectsAttr BridgedEffectsAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedStringRef cCustomString,
    BridgedSourceLoc cCustomStringLoc) {
  return new (cContext.unbridged())
      EffectsAttr(cAtLoc.unbridged(), cRange.unbridged(),
                  cCustomString.unbridged(), cCustomStringLoc.unbridged());
}

static ExclusivityAttr::Mode unbridged(BridgedExclusivityAttrMode mode) {
  switch (mode) {
  case BridgedExclusivityAttrModeChecked:
    return ExclusivityAttr::Checked;
  case BridgedExclusivityAttrModeUnchecked:
    return ExclusivityAttr::Unchecked;
  }
  llvm_unreachable("unhandled enum value");
}
BridgedExclusivityAttr BridgedExclusivityAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedExclusivityAttrMode cMode) {
  return new (cContext.unbridged())
      ExclusivityAttr(cAtLoc.unbridged(), cRange.unbridged(), unbridged(cMode));
}

static ExposureKind unbridged(BridgedExposureKind kind) {
  switch (kind) {
  case BridgedExposureKindCxx:
    return ExposureKind::Cxx;
  case BridgedExposureKindWasm:
    return ExposureKind::Wasm;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedExposeAttr BridgedExposeAttr_createParsed(BridgedASTContext cContext,
                                                 BridgedSourceLoc cAtLoc,
                                                 BridgedSourceRange cRange,
                                                 BridgedStringRef cName,
                                                 BridgedExposureKind cKind) {
  return new (cContext.unbridged())
      ExposeAttr(cName.unbridged(), cAtLoc.unbridged(), cRange.unbridged(),
                 unbridged(cKind), /*Implicit=*/false);
}

static ExternKind unbridged(BridgedExternKind kind) {
  switch (kind) {
  case BridgedExternKindC:
    return ExternKind::C;
  case BridgedExternKindWasm:
    return ExternKind::Wasm;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedExternAttr BridgedExternAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedSourceLoc cLParenLoc,
    BridgedSourceLoc cRParenLoc, BridgedExternKind cKind,
    BridgedStringRef cModuleName, BridgedStringRef cName) {
  std::optional<StringRef> moduleName = cModuleName.unbridged();
  if (moduleName->empty())
    moduleName = std::nullopt;

  std::optional<StringRef> name = cName.unbridged();
  if (name->empty())
    name = std::nullopt;

  return new (cContext.unbridged())
      ExternAttr(moduleName, name, cAtLoc.unbridged(), cLParenLoc.unbridged(),
                 cRParenLoc.unbridged(), cRange.unbridged(), unbridged(cKind),
                 /*Implicit=*/false);
}

BridgedImplementsAttr BridgedImplementsAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedTypeRepr cProtocolType,
    BridgedDeclNameRef cMemberName, BridgedDeclNameLoc cMemberNameLoc) {
  return ImplementsAttr::create(cContext.unbridged(), cAtLoc.unbridged(),
                                cRange.unbridged(), cProtocolType.unbridged(),
                                cMemberName.unbridged().getFullName(),
                                cMemberNameLoc.unbridged());
}

static InlineKind unbridged(BridgedInlineKind kind) {
  switch (kind) {
  case BridgedInlineKindNever:
    return InlineKind::Never;
  case BridgedInlineKindAlways:
    return InlineKind::Always;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedInlineAttr BridgedInlineAttr_createParsed(BridgedASTContext cContext,
                                                 BridgedSourceLoc cAtLoc,
                                                 BridgedSourceRange cRange,
                                                 BridgedInlineKind cKind) {
  return new (cContext.unbridged())
      InlineAttr(cAtLoc.unbridged(), cRange.unbridged(), unbridged(cKind));
}

static swift::ParsedLifetimeDependenceKind
unbridged(BridgedParsedLifetimeDependenceKind kind) {
  switch (kind) {
  case BridgedParsedLifetimeDependenceKindDefault:
    return swift::ParsedLifetimeDependenceKind::Default;
  case BridgedParsedLifetimeDependenceKindScope:
    return swift::ParsedLifetimeDependenceKind::Scope;
  case BridgedParsedLifetimeDependenceKindInherit:
    return swift::ParsedLifetimeDependenceKind::Inherit;
  }
  llvm_unreachable("unhandled enum value");
}

swift::LifetimeDescriptor BridgedLifetimeDescriptor::unbridged() {
  switch (kind) {
  case DescriptorKind::Named:
    return LifetimeDescriptor::forNamed(
        value.name.unbridged(), ::unbridged(dependenceKind), loc.unbridged());
  case DescriptorKind::Ordered:
    return LifetimeDescriptor::forOrdered(
        value.index, ::unbridged(dependenceKind), loc.unbridged());
  case DescriptorKind::Self:
    return LifetimeDescriptor::forSelf(::unbridged(dependenceKind),
                                       loc.unbridged());
  }
  llvm_unreachable("unhandled enum value");
}

static BridgedLifetimeEntry BridgedLifetimeEntry_createParsedImpl(
    BridgedASTContext cContext, BridgedSourceRange cRange,
    BridgedArrayRef cSources,
    std::optional<BridgedLifetimeDescriptor> cTarget) {
  SmallVector<LifetimeDescriptor> sources;
  for (auto cSource : cSources.unbridged<BridgedLifetimeDescriptor>())
    sources.push_back(cSource.unbridged());
  std::optional<LifetimeDescriptor> target;
  if (cTarget)
    target = cTarget->unbridged();

  return LifetimeEntry::create(cContext.unbridged(), cRange.Start.unbridged(),
                               cRange.End.unbridged(), sources, target);
}

BridgedLifetimeEntry
BridgedLifetimeEntry_createParsed(BridgedASTContext cContext,
                                  BridgedSourceRange cRange,
                                  BridgedArrayRef cSources) {
  return BridgedLifetimeEntry_createParsedImpl(cContext, cRange, cSources,
                                               std::nullopt);
}

BridgedLifetimeEntry BridgedLifetimeEntry_createParsed(
    BridgedASTContext cContext, BridgedSourceRange cRange,
    BridgedArrayRef cSources, BridgedLifetimeDescriptor cTarget) {
  return BridgedLifetimeEntry_createParsedImpl(cContext, cRange, cSources,
                                               cTarget);
}

BridgedLifetimeAttr BridgedLifetimeAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedLifetimeEntry cEntry) {
  return LifetimeAttr::create(cContext.unbridged(), cAtLoc.unbridged(),
                              cRange.unbridged(), /*implicit=*/false,
                              cEntry.unbridged());
}

BridgedMacroRole BridgedMacroRole_fromString(BridgedStringRef str) {
  // Match the role string to the known set of roles.
  auto role =
      llvm::StringSwitch<std::optional<BridgedMacroRole>>(str.unbridged())
#define MACRO_ROLE(Name, Description) .Case(Description, BridgedMacroRole##Name)
#include "swift/Basic/MacroRoles.def"
          .Default(std::nullopt);
  return role.has_value() ? *role : BridgedMacroRoleNone;
}

MacroSyntax unbridge(BridgedMacroSyntax cSyntax) {
  switch (cSyntax) {
  case BridgedMacroSyntaxAttached:
    return MacroSyntax::Attached;
  case BridgedMacroSyntaxFreestanding:
    return MacroSyntax::Freestanding;
  }
}

BridgedMacroRoleAttr BridgedMacroRoleAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedMacroSyntax cSyntax,
    BridgedSourceLoc cLParenLoc, BridgedMacroRole cRole, BridgedArrayRef cNames,
    BridgedArrayRef cConformances, BridgedSourceLoc cRParenLoc) {
  SmallVector<MacroIntroducedDeclName, 2> names;
  for (auto &n : cNames.unbridged<BridgedMacroIntroducedDeclName>())
    names.push_back(n.unbridged());

  SmallVector<Expr *, 2> conformances;
  for (auto &t : cConformances.unbridged<BridgedExpr>())
    conformances.push_back(t.unbridged());

  return MacroRoleAttr::create(
      cContext.unbridged(), cAtLoc.unbridged(), cRange.unbridged(),
      unbridge(cSyntax), cLParenLoc.unbridged(), unbridge(cRole), names,
      conformances, cRParenLoc.unbridged(), /*implicit=*/false);
}

BridgedOriginallyDefinedInAttr BridgedOriginallyDefinedInAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedStringRef cModuleName,
    BridgedPlatformKind cPlatform, BridgedVersionTuple cVersion) {
  return new (cContext.unbridged()) OriginallyDefinedInAttr(
      cAtLoc.unbridged(), cRange.unbridged(), cModuleName.unbridged(),
      unbridge(cPlatform), cVersion.unbridged(),
      /*Implicit=*/false);
}

BridgedStorageRestrictionsAttr BridgedStorageRestrictionsAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedArrayRef cInitializes,
    BridgedArrayRef cAccesses) {
  ASTContext &context = cContext.unbridged();

  ArrayRef<Identifier> initializes =
      cContext.unbridged().AllocateTransform<Identifier>(
          cInitializes.unbridged<BridgedIdentifier>(),
          [](auto &e) { return e.unbridged(); });
  ArrayRef<Identifier> accesses =
      cContext.unbridged().AllocateTransform<Identifier>(
          cAccesses.unbridged<BridgedIdentifier>(),
          [](auto &e) { return e.unbridged(); });

  return StorageRestrictionsAttr::create(
      context, cAtLoc.unbridged(), cRange.unbridged(), initializes, accesses);
}

BridgedSwiftNativeObjCRuntimeBaseAttr
BridgedSwiftNativeObjCRuntimeBaseAttr_createParsed(BridgedASTContext cContext,
                                                   BridgedSourceLoc cAtLoc,
                                                   BridgedSourceRange cRange,
                                                   BridgedIdentifier cName) {
  return new (cContext.unbridged())
      SwiftNativeObjCRuntimeBaseAttr(cName.unbridged(), cAtLoc.unbridged(),
                                     cRange.unbridged(), /*Implicit=*/false);
}

static NonSendableKind unbridged(BridgedNonSendableKind kind) {
  switch (kind) {
  case BridgedNonSendableKindSpecific:
    return NonSendableKind::Specific;
  case BridgedNonSendableKindAssumed:
    return NonSendableKind::Assumed;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedNonSendableAttr BridgedNonSendableAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedNonSendableKind cKind) {
  return new (cContext.unbridged())
      NonSendableAttr(cAtLoc.unbridged(), cRange.unbridged(), unbridged(cKind));
}

BridgedNonisolatedAttr
BridgedNonisolatedAttr_createParsed(BridgedASTContext cContext,
                                    BridgedSourceLoc cAtLoc,
                                    BridgedSourceRange cRange, bool isUnsafe) {
  return new (cContext.unbridged()) NonisolatedAttr(
      cAtLoc.unbridged(), cRange.unbridged(), isUnsafe, /*implicit=*/false);
}

BridgedObjCAttr
BridgedObjCAttr_createParsedUnnamed(BridgedASTContext cContext,
                                    BridgedSourceLoc cAtLoc,
                                    BridgedSourceLoc cAttrNameLoc) {
  return ObjCAttr::createUnnamed(cContext.unbridged(), cAtLoc.unbridged(),
                                 cAttrNameLoc.unbridged());
}

BridgedObjCAttr BridgedObjCAttr_createParsedNullary(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceLoc cAttrNameLoc, BridgedSourceLoc cLParenLoc,
    BridgedSourceLoc cNameLoc, BridgedIdentifier cName,
    BridgedSourceLoc cRParenLoc) {
  return ObjCAttr::createNullary(cContext.unbridged(), cAtLoc.unbridged(),
                                 cAttrNameLoc.unbridged(),
                                 cLParenLoc.unbridged(), cNameLoc.unbridged(),
                                 cName.unbridged(), cRParenLoc.unbridged());
}

BridgedObjCAttr BridgedObjCAttr_createParsedSelector(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceLoc cAttrNameLoc, BridgedSourceLoc cLParenLoc,
    BridgedArrayRef cNameLocs, BridgedArrayRef cNames,
    BridgedSourceLoc cRParenLoc) {
  SmallVector<SourceLoc> nameLocs;
  for (auto elem : cNameLocs.unbridged<BridgedSourceLoc>())
    nameLocs.push_back(elem.unbridged());
  SmallVector<Identifier> names;
  for (auto elem : cNames.unbridged<BridgedIdentifier>())
    names.push_back(elem.unbridged());

  return ObjCAttr::createSelector(
      cContext.unbridged(), cAtLoc.unbridged(), cAttrNameLoc.unbridged(),
      cLParenLoc.unbridged(), nameLocs, names, cRParenLoc.unbridged());
}

BridgedObjCImplementationAttr BridgedObjCImplementationAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedIdentifier cName, bool isEarlyAdopter) {
  return new (cContext.unbridged())
      ObjCImplementationAttr(cName.unbridged(), cAtLoc.unbridged(),
                             cRange.unbridged(), isEarlyAdopter);
}

BridgedObjCRuntimeNameAttr BridgedObjCRuntimeNameAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedIdentifier cName) {
  return new (cContext.unbridged())
      ObjCRuntimeNameAttr(cName.unbridged().str(), cAtLoc.unbridged(),
                          cRange.unbridged(), /*Implicit=*/false);
}

static OptimizationMode unbridged(BridgedOptimizationMode mode) {
  switch (mode) {
  case BridgedOptimizationModeForSpeed:
    return OptimizationMode::ForSpeed;
  case BridgedOptimizationModeForSize:
    return OptimizationMode::ForSize;
  case BridgedOptimizationModeNoOptimization:
    return OptimizationMode::NoOptimization;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedOptimizeAttr BridgedOptimizeAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedOptimizationMode cMode) {

  return new (cContext.unbridged())
      OptimizeAttr(cAtLoc.unbridged(), cRange.unbridged(), unbridged(cMode));
}

BridgedPrivateImportAttr BridgedPrivateImportAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceLoc cAttrNameLoc, BridgedSourceLoc cLParenLoc,
    BridgedStringRef cFileName, BridgedSourceLoc cRParenLoc) {
  return PrivateImportAttr::create(
      cContext.unbridged(), cAtLoc.unbridged(), cAttrNameLoc.unbridged(),
      cLParenLoc.unbridged(), cFileName.unbridged(), cRParenLoc.unbridged());
}

BridgedProjectedValuePropertyAttr
BridgedProjectedValuePropertyAttr_createParsed(BridgedASTContext cContext,
                                               BridgedSourceLoc cAtLoc,
                                               BridgedSourceRange cRange,
                                               BridgedIdentifier cName) {
  return new (cContext.unbridged())
      ProjectedValuePropertyAttr(cName.unbridged(), cAtLoc.unbridged(),
                                 cRange.unbridged(), /*Implicit=*/false);
}

BridgedRawDocCommentAttr
BridgedRawDocCommentAttr_createParsed(BridgedASTContext cContext,
                                      BridgedCharSourceRange cRange) {
  return new (cContext.unbridged()) RawDocCommentAttr(cRange.unbridged());
}

ReferenceOwnership unbridged(BridgedReferenceOwnership kind) {
  switch (kind) {
  case BridgedReferenceOwnershipStrong:
    return ReferenceOwnership::Strong;
  case BridgedReferenceOwnershipWeak:
    return ReferenceOwnership::Weak;
  case BridgedReferenceOwnershipUnowned:
    return ReferenceOwnership::Unowned;
  case BridgedReferenceOwnershipUnmanaged:
    return ReferenceOwnership::Unmanaged;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedReferenceOwnershipAttr BridgedReferenceOwnershipAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedReferenceOwnership cKind) {
  return new (cContext.unbridged())
      ReferenceOwnershipAttr(cRange.unbridged(), unbridged(cKind));
}

BridgedSectionAttr BridgedSectionAttr_createParsed(BridgedASTContext cContext,
                                                   BridgedSourceLoc cAtLoc,
                                                   BridgedSourceRange cRange,
                                                   BridgedStringRef cName) {
  return new (cContext.unbridged())
      SectionAttr(cName.unbridged(), cAtLoc.unbridged(), cRange.unbridged(),
                  /*Implicit=*/false);
}

BridgedSemanticsAttr BridgedSemanticsAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedStringRef cValue) {
  return new (cContext.unbridged())
      SemanticsAttr(cValue.unbridged(), cAtLoc.unbridged(), cRange.unbridged(),
                    /*Implicit=*/false);
}

BridgedSetterAccessAttr
BridgedSetterAccessAttr_createParsed(BridgedASTContext cContext,
                                     BridgedSourceRange cRange,
                                     BridgedAccessLevel cAccessLevel) {
  return new (cContext.unbridged()) SetterAccessAttr(
      /*atLoc=*/{}, cRange.unbridged(), unbridge(cAccessLevel).value());
}

static SpecializeAttr::SpecializationKind
unbridge(BridgedSpecializationKind kind) {
  switch (kind) {
  case BridgedSpecializationKindFull:
    return SpecializeAttr::SpecializationKind::Full;
  case BridgedSpecializationKindPartial:
    return SpecializeAttr::SpecializationKind::Partial;
  }
  llvm_unreachable("unhandled kind");
}

BridgedSpecializeAttr BridgedSpecializeAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedNullableTrailingWhereClause cWhereClause,
    bool exported, BridgedSpecializationKind cKind,
    BridgedDeclNameRef cTargetFunction, BridgedArrayRef cSPIGroups,
    BridgedArrayRef cAvailableAttrs) {
  SmallVector<Identifier, 2> spiGroups;
  for (auto bridging : cSPIGroups.unbridged<BridgedIdentifier>())
    spiGroups.push_back(bridging.unbridged());
  SmallVector<AvailableAttr *, 2> availableAttrs;
  for (auto bridging : cAvailableAttrs.unbridged<BridgedAvailableAttr>())
    availableAttrs.push_back(bridging.unbridged());

  return SpecializeAttr::create(
      cContext.unbridged(), cAtLoc.unbridged(), cRange.unbridged(),
      cWhereClause.unbridged(), exported, unbridge(cKind),
      cTargetFunction.unbridged(), spiGroups, availableAttrs);
}

BridgedSPIAccessControlAttr BridgedSPIAccessControlAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedIdentifier cSPIGroupName) {

  return SPIAccessControlAttr::create(cContext.unbridged(), cAtLoc.unbridged(),
                                      cRange.unbridged(),
                                      cSPIGroupName.unbridged());
}

BridgedSILGenNameAttr BridgedSILGenNameAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedStringRef cName, bool isRaw) {
  return new (cContext.unbridged())
      SILGenNameAttr(cName.unbridged(), isRaw, cAtLoc.unbridged(),
                     cRange.unbridged(), /*Implicit=*/false);
}

BridgedUnavailableFromAsyncAttr BridgedUnavailableFromAsyncAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceRange cRange, BridgedStringRef cMessage) {
  return new (cContext.unbridged())
      UnavailableFromAsyncAttr(cMessage.unbridged(), cAtLoc.unbridged(),
                               cRange.unbridged(), /*implicit=*/false);
}

static ExecutionKind unbridged(BridgedExecutionKind kind) {
  switch (kind) {
  case BridgedExecutionKindConcurrent:
    return ExecutionKind::Concurrent;
  case BridgedExecutionKindCaller:
    return ExecutionKind::Caller;
  }
  llvm_unreachable("unhandled enum value");
}

BridgedExecutionAttr BridgedExecutionAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc atLoc,
    BridgedSourceRange range, BridgedExecutionKind behavior) {
  return new (cContext.unbridged())
      ExecutionAttr(atLoc.unbridged(), range.unbridged(),
                    unbridged(behavior), /*implicit=*/false);
}
