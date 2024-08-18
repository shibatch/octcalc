#include <vector>
#include <unordered_map>
#include <regex>
#include <string>

#include <tlfloat/tlfloat.h>

using namespace std;

namespace octcore {
  struct Token {
    string first, second;
    int pos = 0;
    Token(const string& f, const string& s, int p = 0) : first(f), second(s), pos(p) {}
    Token(const char *f, const char *s, int p = 0) : first(f), second(s), pos(p) {}
  };

  class Tokenizer {
    string str;
    vector<pair<string, regex>> tokenDefs;
    vector<Token> pushedBack;

  public:
    size_t pos = 0, lastpos = 0;

    Tokenizer(const string &in, const vector<pair<string, string>> &tokenDefs_) : str(in) {
      for(auto a : tokenDefs_) tokenDefs.push_back(pair<string, regex>(a.first, regex("(" + a.second + ").*")));
    }

    Token next() {
      if (pushedBack.size() != 0) { Token t = pushedBack.back(); pushedBack.pop_back(); return t; }
      Token ret("", "");
      int maxnp = 0, sp = 0;

      while(isspace(char(str[sp]))) sp++;
      str = str.substr(sp);
      lastpos = pos + sp;

      if (str != "") ret = Token(string("character '") + str[0] + "'", "");

      for(auto a : tokenDefs) {
	smatch m;
	if (!regex_match(str, m, a.second)) continue;
	int np = (int)m.position(1) + (int)m[1].str().size();
	if (np > maxnp) { maxnp = np; ret = Token(a.first, m[1].str()); }
      }

      if (maxnp > 0) { pos += sp + maxnp; str = str.substr(maxnp); }

      ret.pos = (int)lastpos;

      return ret;
    }

    void pushBack(const Token &p) { pushedBack.push_back(p); }
  };

  class OctCore {
    vector<pair<string, tlfloat_octuple>> L0p(class Tokenizer& tk, vector<pair<string, tlfloat_octuple>> v);
    pair<string, tlfloat_octuple> L1(class Tokenizer& tk), L0(class Tokenizer& tk);
    pair<string, tlfloat_octuple> L2p(class Tokenizer& tk, pair<string, tlfloat_octuple> lhs);
    pair<string, tlfloat_octuple> L2(class Tokenizer& tk) { return L2p(tk, L1(tk)); } // L2 ::= L1 L2p
    pair<string, tlfloat_octuple> L3p(class Tokenizer& tk, pair<string, tlfloat_octuple> lhs);
    pair<string, tlfloat_octuple> L3(class Tokenizer& tk) { return L3p(tk, L2(tk)); } // L3 ::= L2 L3p
    pair<string, tlfloat_octuple> L4p(class Tokenizer& tk, pair<string, tlfloat_octuple> lhs);
    pair<string, tlfloat_octuple> L4(class Tokenizer& tk) { return L4p(tk, L3(tk)); } // L4 ::= L3 L4p
    pair<string, tlfloat_octuple> L5p(class Tokenizer& tk, pair<string, tlfloat_octuple> lhs);
    pair<string, tlfloat_octuple> L5(class Tokenizer& tk) { return L5p(tk, L4(tk)); } // L5 ::= L4 L5p
    pair<string, tlfloat_octuple> L6p(class Tokenizer& tk, pair<string, tlfloat_octuple> lhs);
    pair<string, tlfloat_octuple> L6(class Tokenizer& tk) { return L6p(tk, L5(tk)); } // L6 ::= L5 L6p
    pair<string, tlfloat_octuple> L7p(class Tokenizer& tk, pair<string, tlfloat_octuple> lhs);
    pair<string, tlfloat_octuple> L7(class Tokenizer& tk) { return L7p(tk, L6(tk)); } // L7 ::= L6 L7p
    pair<string, tlfloat_octuple> L8(class Tokenizer& tk), L8p(class Tokenizer& tk, const pair<string, tlfloat_octuple> &lval);
    pair<string, tlfloat_octuple> LTop(class Tokenizer& tk) { return L8(tk); }

    unordered_map<string, tlfloat_octuple> varMap;
  public:
    pair<string, tlfloat_octuple> execute(const string &str);
    void clear() { varMap.clear(); }
  };
}
