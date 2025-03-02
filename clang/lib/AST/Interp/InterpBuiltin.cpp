//===--- InterpBuiltin.cpp - Interpreter for the constexpr VM ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "Boolean.h"
#include "Interp.h"
#include "PrimType.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/TargetInfo.h"

namespace clang {
namespace interp {

template <typename T>
static T getParam(const InterpFrame *Frame, unsigned Index) {
  assert(Frame->getFunction()->getNumParams() > Index);
  unsigned Offset = Frame->getFunction()->getParamOffset(Index);
  return Frame->getParam<T>(Offset);
}

static bool interp__builtin_strcmp(InterpState &S, CodePtr OpPC,
                                   const InterpFrame *Frame) {
  const Pointer &A = getParam<Pointer>(Frame, 0);
  const Pointer &B = getParam<Pointer>(Frame, 1);

  if (!CheckLive(S, OpPC, A, AK_Read) || !CheckLive(S, OpPC, B, AK_Read))
    return false;

  assert(A.getFieldDesc()->isPrimitiveArray());
  assert(B.getFieldDesc()->isPrimitiveArray());

  unsigned IndexA = A.getIndex();
  unsigned IndexB = B.getIndex();
  int32_t Result = 0;
  for (;; ++IndexA, ++IndexB) {
    const Pointer &PA = A.atIndex(IndexA);
    const Pointer &PB = B.atIndex(IndexB);
    if (!CheckRange(S, OpPC, PA, AK_Read) ||
        !CheckRange(S, OpPC, PB, AK_Read)) {
      return false;
    }
    uint8_t CA = PA.deref<uint8_t>();
    uint8_t CB = PB.deref<uint8_t>();

    if (CA > CB) {
      Result = 1;
      break;
    } else if (CA < CB) {
      Result = -1;
      break;
    }
    if (CA == 0 || CB == 0)
      break;
  }

  S.Stk.push<Integral<32, true>>(Integral<32, true>::from(Result));
  return true;
}

static bool interp__builtin_nan(InterpState &S, CodePtr OpPC,
                                const InterpFrame *Frame, const Function *F,
                                bool Signaling) {
  const Pointer &Arg = getParam<Pointer>(Frame, 0);

  if (!CheckLoad(S, OpPC, Arg))
    return false;

  assert(Arg.getFieldDesc()->isPrimitiveArray());

  // Convert the given string to an integer using StringRef's API.
  llvm::APInt Fill;
  std::string Str;
  assert(Arg.getNumElems() >= 1);
  for (unsigned I = 0;; ++I) {
    const Pointer &Elem = Arg.atIndex(I);

    if (!CheckLoad(S, OpPC, Elem))
      return false;

    if (Elem.deref<int8_t>() == 0)
      break;

    Str += Elem.deref<char>();
  }

  // Treat empty strings as if they were zero.
  if (Str.empty())
    Fill = llvm::APInt(32, 0);
  else if (StringRef(Str).getAsInteger(0, Fill))
    return false;

  const llvm::fltSemantics &TargetSemantics =
      S.getCtx().getFloatTypeSemantics(F->getDecl()->getReturnType());

  Floating Result;
  if (S.getCtx().getTargetInfo().isNan2008()) {
    if (Signaling)
      Result = Floating(
          llvm::APFloat::getSNaN(TargetSemantics, /*Negative=*/false, &Fill));
    else
      Result = Floating(
          llvm::APFloat::getQNaN(TargetSemantics, /*Negative=*/false, &Fill));
  } else {
    // Prior to IEEE 754-2008, architectures were allowed to choose whether
    // the first bit of their significand was set for qNaN or sNaN. MIPS chose
    // a different encoding to what became a standard in 2008, and for pre-
    // 2008 revisions, MIPS interpreted sNaN-2008 as qNan and qNaN-2008 as
    // sNaN. This is now known as "legacy NaN" encoding.
    if (Signaling)
      Result = Floating(
          llvm::APFloat::getQNaN(TargetSemantics, /*Negative=*/false, &Fill));
    else
      Result = Floating(
          llvm::APFloat::getSNaN(TargetSemantics, /*Negative=*/false, &Fill));
  }

  S.Stk.push<Floating>(Result);
  return true;
}

static bool interp__builtin_inf(InterpState &S, CodePtr OpPC,
                                const InterpFrame *Frame, const Function *F) {
  const llvm::fltSemantics &TargetSemantics =
      S.getCtx().getFloatTypeSemantics(F->getDecl()->getReturnType());

  S.Stk.push<Floating>(Floating::getInf(TargetSemantics));
  return true;
}

static bool interp__builtin_copysign(InterpState &S, CodePtr OpPC,
                                     const InterpFrame *Frame,
                                     const Function *F) {
  const Floating &Arg1 = getParam<Floating>(Frame, 0);
  const Floating &Arg2 = getParam<Floating>(Frame, 1);

  APFloat Copy = Arg1.getAPFloat();
  Copy.copySign(Arg2.getAPFloat());
  S.Stk.push<Floating>(Floating(Copy));

  return true;
}

static bool interp__builtin_fmin(InterpState &S, CodePtr OpPC,
                                 const InterpFrame *Frame, const Function *F) {
  const Floating &LHS = getParam<Floating>(Frame, 0);
  const Floating &RHS = getParam<Floating>(Frame, 1);

  Floating Result;

  // When comparing zeroes, return -0.0 if one of the zeroes is negative.
  if (LHS.isZero() && RHS.isZero() && RHS.isNegative())
    Result = RHS;
  else if (LHS.isNan() || RHS < LHS)
    Result = RHS;
  else
    Result = LHS;

  S.Stk.push<Floating>(Result);
  return true;
}

bool InterpretBuiltin(InterpState &S, CodePtr OpPC, const Function *F) {
  InterpFrame *Frame = S.Current;
  APValue Dummy;

  switch (F->getBuiltinID()) {
  case Builtin::BI__builtin_is_constant_evaluated:
    S.Stk.push<Boolean>(Boolean::from(S.inConstantContext()));
    return Ret<PT_Bool>(S, OpPC, Dummy);
  case Builtin::BI__builtin_assume:
    return RetVoid(S, OpPC, Dummy);
  case Builtin::BI__builtin_strcmp:
    if (interp__builtin_strcmp(S, OpPC, Frame))
      return Ret<PT_Sint32>(S, OpPC, Dummy);
    break;
  case Builtin::BI__builtin_nan:
  case Builtin::BI__builtin_nanf:
  case Builtin::BI__builtin_nanl:
  case Builtin::BI__builtin_nanf16:
  case Builtin::BI__builtin_nanf128:
    if (interp__builtin_nan(S, OpPC, Frame, F, /*Signaling=*/false))
      return Ret<PT_Float>(S, OpPC, Dummy);
    break;
  case Builtin::BI__builtin_nans:
  case Builtin::BI__builtin_nansf:
  case Builtin::BI__builtin_nansl:
  case Builtin::BI__builtin_nansf16:
  case Builtin::BI__builtin_nansf128:
    if (interp__builtin_nan(S, OpPC, Frame, F, /*Signaling=*/true))
      return Ret<PT_Float>(S, OpPC, Dummy);
    break;

  case Builtin::BI__builtin_huge_val:
  case Builtin::BI__builtin_huge_valf:
  case Builtin::BI__builtin_huge_vall:
  case Builtin::BI__builtin_huge_valf16:
  case Builtin::BI__builtin_huge_valf128:
  case Builtin::BI__builtin_inf:
  case Builtin::BI__builtin_inff:
  case Builtin::BI__builtin_infl:
  case Builtin::BI__builtin_inff16:
  case Builtin::BI__builtin_inff128:
    if (interp__builtin_inf(S, OpPC, Frame, F))
      return Ret<PT_Float>(S, OpPC, Dummy);
    break;
  case Builtin::BI__builtin_copysign:
  case Builtin::BI__builtin_copysignf:
  case Builtin::BI__builtin_copysignl:
  case Builtin::BI__builtin_copysignf128:
    if (interp__builtin_copysign(S, OpPC, Frame, F))
      return Ret<PT_Float>(S, OpPC, Dummy);
    break;

  case Builtin::BI__builtin_fmin:
  case Builtin::BI__builtin_fminf:
  case Builtin::BI__builtin_fminl:
  case Builtin::BI__builtin_fminf16:
  case Builtin::BI__builtin_fminf128:
    if (interp__builtin_fmin(S, OpPC, Frame, F))
      return Ret<PT_Float>(S, OpPC, Dummy);
    break;

  default:
    return false;
  }

  return false;
}

} // namespace interp
} // namespace clang
