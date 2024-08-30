#include <iostream>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <cctype>
#include <cmath>

#include "octcore.hpp"
#include "rng.hpp"

using namespace octcore;

namespace {
  tlfloat_octuple uplus(tlfloat_octuple a) { return a; }
  tlfloat_octuple uminus(tlfloat_octuple a) { return -a; }
  tlfloat_octuple unot(tlfloat_octuple a) { return ~tlfloat_int128_t(a); }
  tlfloat_octuple bmul(tlfloat_octuple x, tlfloat_octuple y) { return x * y; }
  tlfloat_octuple bdiv(tlfloat_octuple x, tlfloat_octuple y) { return x / y; }
  tlfloat_octuple badd(tlfloat_octuple x, tlfloat_octuple y) { return x + y; }
  tlfloat_octuple bsub(tlfloat_octuple x, tlfloat_octuple y) { return x - y; }
  tlfloat_octuple bshl(tlfloat_octuple x, tlfloat_octuple y) { return tlfloat_int128_t(x) << int(y); }
  tlfloat_octuple bshr(tlfloat_octuple x, tlfloat_octuple y) { return tlfloat_int128_t(x) >> int(y); }
  tlfloat_octuple band(tlfloat_octuple x, tlfloat_octuple y) { return tlfloat_int128_t(x) & tlfloat_int128_t(y); }
  tlfloat_octuple bxor(tlfloat_octuple x, tlfloat_octuple y) { return tlfloat_int128_t(x) ^ tlfloat_int128_t(y); }
  tlfloat_octuple bor (tlfloat_octuple x, tlfloat_octuple y) { return tlfloat_int128_t(x) | tlfloat_int128_t(y); }
  tlfloat_octuple bsubst(tlfloat_octuple x, tlfloat_octuple y) { return y; }
  tlfloat_octuple ldexp_(tlfloat_octuple x, tlfloat_octuple y) { return tlfloat_ldexpo(x, int(y)); }
  tlfloat_octuple gcd(tlfloat_octuple x, tlfloat_octuple y) {
    tlfloat_int128_t a = tlfloat_int128_t(x), b = tlfloat_int128_t(y);
    while(b != 0) { auto t = b; b = a % b; a = t; }
    return a;
  }
  tlfloat_octuple lcm(tlfloat_octuple x, tlfloat_octuple y) { return tlfloat_trunco(x) / gcd(x, y) * tlfloat_trunco(y); }

  tlfloat_octuple rnd(tlfloat_octuple x) {
    static TLCG64 lcg64;
    if (x < 0 || x >= 0x1p+64) return NAN;
    uint64_t u = (uint64_t)x;
    return tlfloat_uint128_t(u == 0 ? lcg64.next64() : lcg64.nextLT(u));
  }

  struct Func {
    const int narg;
    tlfloat_octuple (* const func1)(tlfloat_octuple a1), (* const func2)(tlfloat_octuple a1, tlfloat_octuple a2);
    tlfloat_octuple (* const func3)(tlfloat_octuple a1, tlfloat_octuple a2, tlfloat_octuple a3);
  };
}

// L8 ::= L7 L8p
pair<string, tlfloat_octuple> OctCore::L8(Tokenizer& tk) {
  auto lval = L7(tk), rval = L8p(tk, lval);
  if (lval.first.substr(0, 5) == "LVAL:")
    return pair<string, tlfloat_octuple>(lval.first, varMap[lval.first.substr(5)]);
  return lval;
}

// L8p ::= OP L7 L8p | epsilon      OP : = += -= ...
pair<string, tlfloat_octuple> OctCore::L8p(Tokenizer& tk, const pair<string, tlfloat_octuple> &lval) {
  static const unordered_map<string, tlfloat_octuple (* const)(const tlfloat_octuple, const tlfloat_octuple)> opMap = {
    { "=", bsubst }, { "+=", badd }, { "-=", bsub }, { "*=", bmul }, { "/=", bdiv },
    { "%=", tlfloat_fmodo }, { "&=", band }, { "|=", bor }, { "^=", bxor },
    { "<<=", bshl }, { ">>=", bshr },
  };
  auto t0 = tk.next();
  if (opMap.count(t0.first) == 0) {
    tk.pushBack(t0);
    return pair<string, tlfloat_octuple>("", 0);
  }
  if (lval.first.substr(0, 5) != "LVAL:") throw(runtime_error("Expected l-value before assignment operator at column " + to_string(t0.pos)));
  auto rval = L7(tk), rval2 = L8p(tk, rval);
  varMap[lval.first.substr(5)] =
    (*opMap.at(t0.first))(varMap[lval.first.substr(5)],
			  rval.first.substr(0, 5) == "LVAL:" ? varMap[rval.first.substr(5)] : rval.second);
  return pair<string, tlfloat_octuple>(rval.first, varMap[lval.first.substr(5)]);
}

// L7p ::= OP L6 L7p | epsilon      OP : |
pair<string, tlfloat_octuple> OctCore::L7p(Tokenizer& tk, pair<string, tlfloat_octuple> lhs) {
  auto t0 = tk.next();
  if (t0.first == "|") return L7p(tk, pair<string, tlfloat_octuple>("RVAL", bor(lhs.second, L6(tk).second)));
  tk.pushBack(t0);
  return lhs;
}

// L6p ::= OP L5 L6p | epsilon      OP : ^
pair<string, tlfloat_octuple> OctCore::L6p(Tokenizer& tk, pair<string, tlfloat_octuple> lhs) {
  auto t0 = tk.next();
  if (t0.first == "^") return L6p(tk, pair<string, tlfloat_octuple>("RVAL", bxor(lhs.second, L5(tk).second)));
  tk.pushBack(t0);
  return lhs;
}

// L5p ::= OP L4 L5p | epsilon      OP : &
pair<string, tlfloat_octuple> OctCore::L5p(Tokenizer& tk, pair<string, tlfloat_octuple> lhs) {
  auto t0 = tk.next();
  if (t0.first == "&") return L5p(tk, pair<string, tlfloat_octuple>("RVAL", band(lhs.second, L4(tk).second)));
  tk.pushBack(t0);
  return lhs;
}

// L4p ::= OP L3 L4p | epsilon      OP : << >>
pair<string, tlfloat_octuple> OctCore::L4p(Tokenizer& tk, pair<string, tlfloat_octuple> lhs) {
  static const unordered_map<string, tlfloat_octuple (* const)(tlfloat_octuple, tlfloat_octuple)> opMap = {
    { "<<", bshl }, { ">>", bshr },
  };
  auto t0 = tk.next();
  if (opMap.count(t0.first) != 0) {
    auto c0 = L3(tk);
    return L4p(tk, pair<string, tlfloat_octuple>("RVAL", (*opMap.at(t0.first))(lhs.second, c0.second)));
  }
  tk.pushBack(t0);
  return lhs;
}

// L3p ::= OP L2 L3p | epsilon      OP : + -
pair<string, tlfloat_octuple> OctCore::L3p(Tokenizer& tk, pair<string, tlfloat_octuple> lhs) {
  static const unordered_map<string, tlfloat_octuple (* const)(tlfloat_octuple, tlfloat_octuple)> opMap = {
    { "+", badd }, { "-", bsub },
  };
  auto t0 = tk.next();
  if (opMap.count(t0.first) != 0) {
    auto c0 = L2(tk);
    return L3p(tk, pair<string, tlfloat_octuple>("RVAL", (*opMap.at(t0.first))(lhs.second, c0.second)));
  }
  tk.pushBack(t0);
  return lhs;
}

// L2p ::= OP L1 L2p | epsilon      OP : * / %
pair<string, tlfloat_octuple> OctCore::L2p(Tokenizer& tk, pair<string, tlfloat_octuple> lhs) {
  static const unordered_map<string, tlfloat_octuple (* const)(tlfloat_octuple, tlfloat_octuple)> opMap = {
    { "*", bmul }, { "/", bdiv }, { "%", tlfloat_fmodo },
  };
  auto t0 = tk.next();
  if (opMap.count(t0.first) != 0) {
    auto c0 = L1(tk);
    return L2p(tk, pair<string, tlfloat_octuple>("RVAL", (*opMap.at(t0.first))(lhs.second, c0.second)));
  }
  tk.pushBack(t0);
  return lhs;
}

// L1 ::= L0 | OP L1		OP : + - ~
pair<string, tlfloat_octuple> OctCore::L1(Tokenizer& tk) {
  static const unordered_map<string, tlfloat_octuple (* const)(tlfloat_octuple)> opMap = {
    { "+", uplus }, { "-", uminus }, { "~", unot },
  };
  auto t0 = tk.next();
  if (opMap.count(t0.first) != 0) {
    auto c = L1(tk);
    return pair<string, tlfloat_octuple>("RVAL", (*opMap.at(t0.first))(c.second));
  }
  tk.pushBack(t0);
  return L0(tk);
}

// L0p ::= , L8 L0p | epsilon
vector<pair<string, tlfloat_octuple>> OctCore::L0p(Tokenizer& tk, vector<pair<string, tlfloat_octuple>> v) {
  auto t0 = tk.next();
  if (t0.first == ",") { v.push_back(LTop(tk)); return L0p(tk, v); }
  tk.pushBack(t0);
  return v;
}

// L0 ::= FP | ( L8 ) | ID | F | F ( L8 L0p )
pair<string, tlfloat_octuple> OctCore::L0(Tokenizer& tk) {
  static unordered_map<string, Func> funcMap = {
    { "sqrt", Func { 1, tlfloat_sqrto, nullptr, nullptr } }, { "cbrt", Func { 1, tlfloat_cbrto, nullptr, nullptr } },
    { "sin", Func { 1, tlfloat_sino, nullptr, nullptr } }, { "cos", Func { 1, tlfloat_coso, nullptr, nullptr } },
    { "tan", Func { 1, tlfloat_tano, nullptr, nullptr } }, { "asin", Func { 1, tlfloat_asino, nullptr, nullptr } },
    { "acos", Func { 1, tlfloat_acoso, nullptr, nullptr } }, { "atan", Func { 1, tlfloat_atano, nullptr, nullptr } },
    { "sinh", Func { 1, tlfloat_sinho, nullptr, nullptr } }, { "cosh", Func { 1, tlfloat_cosho, nullptr, nullptr } },
    { "tanh", Func { 1, tlfloat_tanho, nullptr, nullptr } }, { "asinh", Func { 1, tlfloat_asinho, nullptr, nullptr } },
    { "acosh", Func { 1, tlfloat_acosho, nullptr, nullptr } }, { "atanh", Func { 1, tlfloat_atanho, nullptr, nullptr } },
    { "log", Func { 1, tlfloat_logo, nullptr, nullptr } }, { "log2", Func { 1, tlfloat_log2o, nullptr, nullptr } },
    { "log10", Func { 1, tlfloat_log10o, nullptr, nullptr } }, { "log1p", Func { 1, tlfloat_log1po, nullptr, nullptr } },
    { "exp", Func { 1, tlfloat_expo, nullptr, nullptr } }, { "exp2", Func { 1, tlfloat_exp2o, nullptr, nullptr } },
    { "exp10", Func { 1, tlfloat_exp10o, nullptr, nullptr } }, { "expm1", Func { 1, tlfloat_expm1o, nullptr, nullptr } },
    { "erf", Func { 1, tlfloat_erfo, nullptr, nullptr } }, { "erfc", Func { 1, tlfloat_erfco, nullptr, nullptr } },
    { "tgamma", Func { 1, tlfloat_tgammao, nullptr, nullptr } }, { "lgamma", Func { 1, tlfloat_lgammao, nullptr, nullptr } },
    { "trunc", Func { 1, tlfloat_trunco, nullptr, nullptr } }, { "floor", Func { 1, tlfloat_flooro, nullptr, nullptr } },
    { "ceil", Func { 1, tlfloat_ceilo, nullptr, nullptr } }, { "round", Func { 1, tlfloat_roundo, nullptr, nullptr } },
    { "rint", Func { 1, tlfloat_rinto, nullptr, nullptr } }, { "fabs", Func { 1, tlfloat_fabso, nullptr, nullptr } }, 
    { "pow", Func { 2, nullptr, tlfloat_powo, nullptr } }, { "atan2", Func { 2, nullptr, tlfloat_atan2o, nullptr } },
    { "hypot", Func { 2, nullptr, tlfloat_hypoto, nullptr } }, { "fdim", Func { 2, nullptr, tlfloat_fdimo, nullptr } },
    { "fmax", Func { 2, nullptr, tlfloat_fmaxo, nullptr } }, { "fmin", Func { 2, nullptr, tlfloat_fmino, nullptr } },
    { "fmod", Func { 2, nullptr, tlfloat_fmodo, nullptr } }, { "remainder", Func { 2, nullptr, tlfloat_remaindero, nullptr } },
    { "copysign", Func { 2, nullptr, tlfloat_copysigno, nullptr } }, { "fma", Func { 3, nullptr, nullptr, tlfloat_fmao } },
    { "ldexp", Func { 2, nullptr, ldexp_, nullptr } }, { "int", Func { 1, tlfloat_trunco, nullptr, nullptr } },
    { "gcd", Func { 2, nullptr, gcd, nullptr } }, { "lcm", Func { 2, nullptr, lcm, nullptr } }, 
    { "rnd", Func { 1, rnd, nullptr, nullptr } },
  };
  static unordered_map<string, tlfloat_octuple> constMap = {
    { "M_E", TLFLOAT_M_Eo }, { "M_LOG2E", TLFLOAT_M_LOG2Eo }, { "M_LOG10E", TLFLOAT_M_LOG10Eo }, { "M_LN2", TLFLOAT_M_LN2o },
    { "M_LN10", TLFLOAT_M_LN10o }, { "M_PI", TLFLOAT_M_PIo }, { "M_PI_2", TLFLOAT_M_PI_2o }, { "M_PI_4", TLFLOAT_M_PI_4o },
    { "M_1_PI", TLFLOAT_M_1_PIo }, { "M_2_PI", TLFLOAT_M_2_PIo }, { "M_2_SQRTPI", TLFLOAT_M_2_SQRTPIo },
    { "M_SQRT2", TLFLOAT_M_SQRT2o }, { "M_SQRT1_2", TLFLOAT_M_SQRT1_2o },
  };

  auto t0 = tk.next();

  if (t0.first == "FP") {
    return pair<string, tlfloat_octuple>("RVAL", tlfloat_strtoo(t0.second.c_str(), nullptr));
  } if (t0.first == "(") {
    auto c = LTop(tk);
    auto t1 = tk.next();
    if (t1.first != ")") throw(runtime_error("')' expected at column " + to_string(t1.pos)));
    return c;
  } else if (t0.first == "ID" && funcMap.count(t0.second) != 0) {
    auto f = funcMap.at(t0.second);
    auto t1 = tk.next();
    if (t1.first != "(") throw(runtime_error("'(' expected at column " + to_string(t1.pos)));
    vector<pair<string, tlfloat_octuple>> c;
    c.push_back(LTop(tk));
    c = L0p(tk, c);
    if ((int)c.size() != f.narg)
      throw(runtime_error(to_string(f.narg) + " argument(s) expected for " + t0.second +
			  " at column " + to_string(t0.pos)));
    auto t2 = tk.next();
    if (t2.first != ")") throw(runtime_error("')' expected at column " + to_string(t2.pos)));
    switch(f.narg) {
    case 1: return pair<string, tlfloat_octuple>("RVAL", (*f.func1)(c[0].second));
    case 2: return pair<string, tlfloat_octuple>("RVAL", (*f.func2)(c[0].second, c[1].second));
    case 3: return pair<string, tlfloat_octuple>("RVAL", (*f.func3)(c[0].second, c[1].second, c[2].second));
    default: abort();
    }
  } else if (t0.first == "ID" && constMap.count(t0.second) != 0) {
    return pair<string, tlfloat_octuple>("RVAL", constMap[t0.second]);
  } else if (t0.first == "ID") {
    return pair<string, tlfloat_octuple>("LVAL:" + t0.second, varMap[t0.second]);
  } else {
    string s = t0.first == "" ? "end of line" : t0.first;
    throw(runtime_error("Unexpected " + s + " at column " + to_string(t0.pos)));
  }
}

pair<string, tlfloat_octuple> OctCore::execute(const string &str) {
  static vector<pair<string, string>> tokenDefs = {
    { "FP", "(0x([0-9a-fA-F]*[.])?[0-9a-fA-F]+([pP][-+]?\\d+)?)|(([0-9]*[.])?[0-9]+([eE][-+]?\\d+)?)|([Ii][Nn][Ff])|([Nn][Aa][Nn])" },
    { "+", "\\+" }, { "-", "-" }, { "*", "\\*" }, { "/", "/" }, { "%", "%" },
    { "<<", "<<" }, { ">>", ">>" }, { "&", "&" }, { "|", "\\|" }, { "^", "\\^" }, { "~", "~" },
    { "+=", "\\+=" }, { "-=", "-=" }, { "*=", "\\*=" }, { "/=", "/=" }, { "%=", "%=" },
    { "&=", "&=" }, { "|=", "\\|=" }, { "^=", "\\^=" }, { "<<=", "<<=" }, { ">>=", ">>=" },
    { "(", "\\(" }, { ")", "\\)" }, { "=", "=" }, { ",", "," }, { "ID", "[a-zA-Z_][a-zA-Z_0-9]*" },
  };

  try {
    Tokenizer tk(str, tokenDefs);
    auto t0 = tk.next();
    if (t0.first == "") return pair<string, tlfloat_octuple>("RVAL", 0);
    tk.pushBack(t0);
    auto c = LTop(tk);
    auto t1 = tk.next();
    if (t1.first != "") throw(runtime_error("Syntax error at column " + to_string(t1.pos)));
    return c;
  } catch(exception &ex) {
    return pair<string, tlfloat_octuple>(string("ERROR:") + ex.what(), 0);
  }
}
