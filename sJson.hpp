#pragma once

#include <map>
#include <string>
#include <typeinfo>
#include <vector>

class sJSONElementNode;

enum class sJSONValueType {
    Value,
    Array,
    Object,
    Null
};

enum class sJSONTokenType {
    BigLeft,
    BigRight,
    MiddleLeft,
    MiddleRight,
    wstring,
    Number,
    Float,
    Boolean,
    Comma,
    Colon,
    Unknown,
    End,
    Null
};

std::map<wchar_t, sJSONTokenType> sJSONTokenMap = {{'{', sJSONTokenType::BigLeft},
                                                {'}', sJSONTokenType::BigRight},
                                                {'[', sJSONTokenType::MiddleLeft},
                                                {']', sJSONTokenType::MiddleRight},
                                                {',', sJSONTokenType::Comma},
                                                {':', sJSONTokenType::Colon}};
std::map<sJSONTokenType, wchar_t> sJSONTokenwstringMap = {
        {sJSONTokenType::BigLeft,     '{'},
        {sJSONTokenType::BigRight,    '}'},
        {sJSONTokenType::MiddleLeft,  '['},
        {sJSONTokenType::MiddleRight, ']'},
        {sJSONTokenType::Comma,       ','},
        {sJSONTokenType::Colon,       ':'}};
std::map<sJSONTokenType, sJSONTokenType> sJSONOppsiteTokenMap = {
        {sJSONTokenType::BigLeft,    sJSONTokenType::BigRight},
        {sJSONTokenType::MiddleLeft, sJSONTokenType::MiddleRight}};

class sJSONValue {
public:
    sJSONValue() = default;

    ~sJSONValue() = default;

public:
    virtual __forceinline const sJSONValueType GetType() const = 0;

    virtual __forceinline const size_t GetHash() const {
        return 0;
    }

    template<class Object>
    Object To() {
        return static_cast<Object>(this);
    }
};

class sJSONNull : public sJSONValue {
public:
    sJSONNull() = default;

    ~sJSONNull() = default;

public:
    __forceinline const sJSONValueType GetType() const override {
        return sJSONValueType::Null;
    }
};

template<class Type>
class sJSONRealValue : public sJSONValue {
public:
    sJSONRealValue(const Type &SetValue) : Value(SetValue) {
    }

    ~sJSONRealValue() = default;

    __forceinline sJSONValueType const GetType() const override {
        return sJSONValueType::Value;
    }

    __forceinline const size_t GetHash() const override {
        return typeid(Type).hash_code();
    }

    static constexpr bool Equal(const sJSONValue *Judgement) {
        if (Judgement->GetType() != sJSONValueType::Value) {
            return false;
        } else {
            return Judgement->GetHash() == typeid(Type).hash_code();
        }
    }

    Type operator*() {
        return Value;
    }

public:
    Type Value;
};

using sJSONwstring = sJSONRealValue<std::wstring>;
using sJSONInt = sJSONRealValue<int>;
using sJSONDouble = sJSONRealValue<double>;
using sJSONBoolean = sJSONRealValue<bool>;

class sJSONArray : public sJSONValue {
public:
    sJSONArray() = default;

    ~sJSONArray() = default;

public:
    __forceinline sJSONValueType const GetType() const override {
        return sJSONValueType::Array;
    }

public:
    std::vector<sJSONValue *> ValueSet;
};

typedef class sJSONElementNode : public sJSONValue {
public:
    sJSONElementNode() {
        Value = nullptr;
    }

    sJSONElementNode(const std::wstring &Tag, sJSONValue *Value) {
        this->Tag = Tag;
        this->Value = Value;
    }

    sJSONElementNode(std::map<std::wstring, sJSONElementNode *> SetChildren) : Children(SetChildren), Value(nullptr) {
    }

    ~sJSONElementNode() {}

public:
    __forceinline sJSONValueType const GetType() const override {
        return sJSONValueType::Object;
    }

    template<class Type>
    Type To() {
        return Value->To<Type>();
    }

public:
    bool IsEmpty() {
        return Value->GetType() == sJSONValueType::Null;
    }

    const std::wstring GetTag() const {
        return Tag;
    }

    void InsertChildrenNode(sJSONElementNode *Node) {
        Children.emplace(Node->Tag, Node);
    }

    const sJSONValue *GetValue() const {
        return Value;
    }

    sJSONValue *operator*() {
        return Value;
    }

public:
    sJSONElementNode *operator[](const std::wstring &ChildrenTag) {
        return Children[ChildrenTag];
    }

public:
    sJSONValue *Value;
    std::wstring Tag;
    std::map<std::wstring, sJSONElementNode *> Children;
} sJSONObject;

class sJSONElementFinder {
public:
    sJSONElementFinder(sJSONElementNode *Ptr) : Value(Ptr) {
    }

    sJSONElementFinder operator[](const std::wstring &Finder) {
        return sJSONElementFinder(Value->operator[](Finder));
    }

    sJSONValue *GetValue() {
        return Value->Value;
    }

    bool IsEmpty() {
        return Value->Value->GetType() == sJSONValueType::Null;
    }

    bool IsArray() {
        return Value->Value->GetType() == sJSONValueType::Array;
    }

    bool IsObject() {
        return Value->Value->GetType() == sJSONValueType::Object;
    }

    bool IsRealValue() {
        return Value->Value->GetType() == sJSONValueType::Value;
    }

    bool Exsits(const std::wstring &Finder) {
        return Value->Children.find(Finder) != Value->Children.end();
    }

    template<class Type>
    Type To() {
        return Value->To<Type>();
    }

    std::map<std::wstring, sJSONElementNode *>::iterator begin() {
        Iterator = Value->Children.begin();

        return Iterator;
    }

    std::map<std::wstring, sJSONElementNode *>::iterator end() {
        return Value->Children.end();
    }

    auto operator++() {
        ++Iterator;

        return *Iterator;
    }

    auto operator++(int) {
        auto Temp(*Iterator);
        ++Iterator;

        return Temp;
    }

    std::vector<sJSONValue *>::iterator ArrayBegin() {
        if (Value->Value->GetType() == sJSONValueType::Array) {
            return Value->Value->To<sJSONArray *>()->ValueSet.begin();
        }

        return std::vector<sJSONValue *>::iterator();
    }

    std::vector<sJSONValue *>::iterator ArrayEnd() {
        if (Value->Value->GetType() == sJSONValueType::Array) {
            return Value->Value->To<sJSONArray *>()->ValueSet.end();
        }

        return std::vector<sJSONValue *>::iterator();
    }

private:
    std::map<std::wstring, sJSONElementNode *>::iterator Iterator;
    sJSONElementNode *Value;
};

class sJSONRootNode {
public:
    sJSONRootNode(sJSONObject *RootObject) : Object(RootObject) {
    }

public:
    sJSONElementFinder operator[](const std::wstring &ChildrenTag) {
        return sJSONElementFinder(Object->operator[](ChildrenTag));
    }

    bool Exsits(const std::wstring &Finder) {
        return Object->Children.find(Finder) != Object->Children.end();
    }

    std::map<std::wstring, sJSONElementNode *>::iterator begin() {
        Iterator = Object->Children.begin();

        return Iterator;
    }

    std::map<std::wstring, sJSONElementNode *>::iterator end() {
        return Object->Children.end();
    }

    auto operator++() {
        ++Iterator;

        return *Iterator;
    }

    auto operator++(int) {
        auto Temp(*Iterator);
        ++Iterator;

        return Temp;
    }

private:
    std::map<std::wstring, sJSONElementNode *>::iterator Iterator;
    sJSONObject *Object;
};

class sJSONParserStatus {
public:
    using ErrorList = std::vector<std::wstring>;
    using Iterator = std::vector<std::wstring>::iterator;

public:
    sJSONParserStatus() = default;

    __forceinline const bool ExsitsError() const {
        return !ErrorInfo.empty();
    }

    std::vector<std::wstring>::iterator begin() {
        NativeIterator = ErrorInfo.begin();
        return NativeIterator;
    }

    std::vector<std::wstring>::iterator end() {
        return ErrorInfo.end();
    }

    std::wstring &operator++() {
        ++NativeIterator;

        return *NativeIterator;
    }

    std::wstring operator++(int) {
        std::wstring Temp(*NativeIterator);
        ++NativeIterator;

        return Temp;
    }

public:
    ErrorList ErrorInfo;
    Iterator NativeIterator;
};

class sJSONLexer {
public:
    sJSONLexer(const std::wstring &Code) : sJSON(Code) {
        Line = 1;
        Iterator = sJSON.begin();
        Rawwstring = true;
    }

public:
    inline std::tuple<std::wstring, sJSONTokenType> operator()() {
        if (Iterator == sJSON.end() || *Iterator == '\0') {
            return {L"", sJSONTokenType::End};
        }
        std::wstring Token;
        bool ExsitsDot = false;
        bool Number = false;
        while (Iterator != sJSON.end()) {
            if (*Iterator == '\n' || *Iterator == '\r') {
                ++Line;
            } else if (*Iterator >= 'a' && *Iterator <= 'z' && *Iterator >= 'A' && *Iterator <= 'Z' &&
                       *Iterator == '_' &&
                       !Number) {
                Token.push_back(*Iterator);
            } else if (((*Iterator >= '0' && *Iterator <= '9') || *Iterator == '.') && (Token.empty() || Number)) {
                Number = true;
                if (*Iterator == '.') {
                    if (ExsitsDot) {
                        ExsitsDot = true;
                        Token.push_back(*Iterator);
                    } else {
                        ParserStatus->ErrorInfo.push_back(L"<Bad number>");
                        return {L"", sJSONTokenType::Unknown};
                    }
                } else {
                    Token.push_back(*Iterator);
                }
            } else if (!Token.empty()) {
                if (!Number) {
                    if (Token == L"true" || Token == L"false") {
                        return {Token, sJSONTokenType::Boolean};
                    }
                    if (Token == L"null") {
                        return {Token, sJSONTokenType::Null};
                    } else {
                        return {Token, sJSONTokenType::Unknown};
                    }
                } else {
                    if (!ExsitsDot) {
                        return {Token, sJSONTokenType::Number};
                    } else {
                        return {Token, sJSONTokenType::Float};
                    }
                }
            } else if (*Iterator == '\"') {
                if (!Rawwstring) {
                    Token.push_back(*Iterator);
                }
                ++Iterator;
                while (Iterator != sJSON.end()) {
                    if (*Iterator == '\"') {
                        if (!Rawwstring) {
                            Token.push_back(*Iterator);
                        }
                        ++Iterator;
                        return {Token, sJSONTokenType::wstring};
                    }
                    if (*Iterator == '\\') {
                        if (Iterator + 1 != sJSON.end() && Rawwstring) {
                            ++Iterator;
                            if (*Iterator == '\"') {
                                Token.push_back('\"');
                            }
                            if (*Iterator == 't') {
                                Token.push_back('\t');
                            }
                            if (*Iterator == 'n') {
                                Token.push_back('\n');
                            }
                            if (*Iterator == 'r') {
                                Token.push_back('\r');
                            }
                        } else {
                            ParserStatus->ErrorInfo.push_back(L"Not match \" of the begin of \"");
                            return {L"", sJSONTokenType::Unknown};
                        }
                    } else {
                        Token.push_back(*Iterator);
                    }

                    ++Iterator;
                }
            } else if (*Iterator != L' ' && *Iterator != L'\t') {
                sJSONTokenType Type = sJSONTokenType::Unknown;
                if (sJSONTokenMap.find(*Iterator) != sJSONTokenMap.end()) {
                    Type = sJSONTokenMap[*Iterator];
                }

                Token.push_back(*Iterator);
                ++Iterator;

                return {Token, Type};
            }

            ++Iterator;
        }

        return {L"", sJSONTokenType::End};
    }

    bool operator*() {
        return Iterator != sJSON.end();
    }

    const size_t GetLine() const {
        return Line;
    }

public:
    bool Rawwstring;

private:
    sJSONParserStatus *ParserStatus;
    std::wstring::iterator Iterator;
    std::wstring sJSON;
    size_t Line;
};

class sJSONParser {
public:
    sJSONParser(std::wstring Code) : RootObject(new sJSONObject()), Lexer(Code) {
    }

public:
    sJSONRootNode Parse() {
        auto Token = Lexer();
        switch (std::get<1>(Token)) {
            case sJSONTokenType::BigLeft: {
                sJSONParser *SubParser = new sJSONParser(
                        FetchContext(sJSONTokenType::BigLeft, sJSONTokenType::BigRight));
                RootObject->Children = SubParser->ParseSet();
                if (SubParser->Status.ExsitsError()) {
                    Status = SubParser->Status;
                    return sJSONRootNode(RootObject);
                }

                break;
            }
            default: {
                Status.ErrorInfo.push_back(L"Unknown token at line " + std::to_wstring(Lexer.GetLine()) + L".");
                return sJSONRootNode(RootObject);
            }
        }

        return sJSONRootNode(RootObject);
    }

private:
    std::map<std::wstring, sJSONElementNode *> ParseSet() {
        std::map<std::wstring, sJSONElementNode *> Set;
        while (*Lexer) {
            auto Token = Lexer();
            switch (std::get<1>(Token)) {
                case sJSONTokenType::wstring: {
                    std::wstring Tag = std::get<0>(Token);
                    Token = Lexer();

                    if (std::get<1>(Token) != sJSONTokenType::Colon) {
                        Status.ErrorInfo.push_back(L"Unknown token at line " + std::to_wstring(Lexer.GetLine()) + L".");
                        return std::map<std::wstring, sJSONElementNode *>();
                    }

                    auto Value = ParseValue();
                    if (Status.ExsitsError()) {
                        return std::map<std::wstring, sJSONElementNode *>();
                    }

                    Set.emplace(Tag, new sJSONElementNode(Tag, Value));
                    if (Value->GetType() == sJSONValueType::Object) {
                        Set[Tag]->Children = static_cast<sJSONObject *>(Value)->Children;
                    }

                    Token = Lexer();
                    if (std::get<1>(Token) != sJSONTokenType::Comma && std::get<1>(Token) != sJSONTokenType::End) {
                        Status.ErrorInfo.push_back(L"Unknown token at line " + std::to_wstring(Lexer.GetLine()) + L".");
                        return std::map<std::wstring, sJSONElementNode *>();
                    }

                    break;
                }
                default: {
                    Status.ErrorInfo.push_back(L"Unknown token at line " + std::to_wstring(Lexer.GetLine()) + L".");
                    return std::map<std::wstring, sJSONElementNode *>();
                }
            }
        }

        return Set;
    }

    sJSONArray *ParseArray() {
        sJSONArray *Array = new sJSONArray;
        while (*Lexer) {
            Array->ValueSet.push_back(ParseValue());
            if (Status.ExsitsError()) {
                return nullptr;
            }
            auto Token = Lexer();

            if (std::get<1>(Token) != sJSONTokenType::End && std::get<1>(Token) != sJSONTokenType::Comma) {
                Status.ErrorInfo.push_back(L"Unknown token at line " + std::to_wstring(Lexer.GetLine()) + L".");
                return nullptr;
            }
        }

        return Array;
    }

    sJSONValue *ParseValue() {
        while (*Lexer) {
            auto Token = Lexer();
            switch (std::get<1>(Token)) {
                case sJSONTokenType::wstring: {
                    return new sJSONRealValue<std::wstring>(std::get<0>(Token));
                }
                case sJSONTokenType::Number: {
                    return new sJSONRealValue<int>(_wtoi(std::get<0>(Token).c_str()));
                }
                case sJSONTokenType::Float: {
                    return new sJSONRealValue<double>(_wtoi(std::get<0>(Token).c_str()));
                }
                case sJSONTokenType::Null: {
                    return new sJSONNull();
                }
                case sJSONTokenType::Boolean: {
                    if (std::get<0>(Token) == L"true") {
                        return new sJSONRealValue<bool>(true);
                    } else {
                        return new sJSONRealValue<bool>(false);
                    }
                }
                case sJSONTokenType::BigLeft: {
                    sJSONParser *SubParser =
                            new sJSONParser(FetchContext(std::get<1>(Token), sJSONOppsiteTokenMap[std::get<1>(Token)]));
                    auto Value = new sJSONObject(SubParser->ParseSet());

                    delete SubParser;

                    return Value;
                }
                case sJSONTokenType::MiddleLeft: {
                    sJSONParser *SubParser =
                            new sJSONParser(FetchContext(std::get<1>(Token), sJSONOppsiteTokenMap[std::get<1>(Token)]));
                    auto Array = SubParser->ParseArray();

                    delete SubParser;

                    return Array;
                }
                default: {
                    Status.ErrorInfo.push_back(L"Unknown token at line " + std::to_wstring(Lexer.GetLine()) + L".");
                    return nullptr;
                }
            }
        }

        Status.ErrorInfo.push_back(L"<Bad sJSON>");
        return nullptr;
    }

private:
    inline std::wstring FetchContext(const sJSONTokenType &Left, const sJSONTokenType &Right) {
        Lexer.Rawwstring = false;
        std::wstring Context;
        int DeepthLevel = 0;
        while (*Lexer) {
            auto Tuple = Lexer();
            if (std::get<1>(Tuple) == Left) {
                ++DeepthLevel;
            }
            if (std::get<1>(Tuple) == Right) {
                --DeepthLevel;
                if (DeepthLevel == -1) {
                    Lexer.Rawwstring = true;
                    return Context;
                }
            }

            Context.append(std::get<0>(Tuple) + L" ");
        }

        Status.ErrorInfo.push_back(L"<Bad sJSON tree>");

        return Context;
    }

private:
    sJSONLexer Lexer;
    sJSONObject *RootObject;
    sJSONParserStatus Status;
};

#define sJSONHelperCountDown(Var) for (int Count = 0; Count < Var; ++Count)

class sJSONWriter {
public:
    static std::wstring WriteJSON(sJSONRootNode &Root, bool Format = true, bool Brackets = true, size_t Level = 1) {
        std::wstring sJSON;
        if (Brackets) {
            if (Format)
                sJSONHelperCountDown(Level) {
                    sJSON.push_back('\t');
                }
            sJSON = L"{";
            if (Format) {
                sJSON.push_back('\n');
            }
        }
        for (auto &Node: Root) {
            if (Format)
                sJSONHelperCountDown(Level) {
                    sJSON.push_back('\t');
                }

            sJSON.append(L"\"" + Node.first + L"\"");
            sJSON.append(L":");
            sJSON.append(WriteValue(Node.second->Value, Format, Brackets, Level));

            sJSON.push_back(',');
            if (Format) {
                sJSON.push_back('\n');
            }
        }

        sJSON.pop_back();
        if (Format) {
            sJSON.pop_back();
            sJSON.push_back('\n');
        }
        if (Brackets) {
            if (Format)
                sJSONHelperCountDown(Level - 1) {
                    sJSON.push_back('\t');
                }
            sJSON.push_back('}');
        }

        return sJSON;
    }

    static std::wstring
    WriteJSON(sJSONElementFinder &Finder, bool Format = true, bool Brackets = true, size_t Level = 1) {
        std::wstring sJSON;
        if (Brackets) {
            if (Format)
                sJSONHelperCountDown(Level) {
                    sJSON.push_back('\t');
                }
            sJSON = L"{";
            if (Format) {
                sJSON.push_back('\n');
            }
        }
        for (auto &Node: Finder) {
            if (Format)
                sJSONHelperCountDown(Level) {
                    sJSON.push_back('\t');
                }

            sJSON.append(Node.first);
            sJSON.append(L":");
            sJSON.append(WriteValue(Node.second->Value, Format, Brackets, Level));

            sJSON.push_back(',');
            if (Format) {
                sJSON.push_back('\n');
            }
        }

        sJSON.pop_back();
        if (Format) {
            sJSON.pop_back();
            sJSON.push_back('\n');
        }
        if (Brackets) {
            if (Format)
                sJSONHelperCountDown(Level - 1) {
                    sJSON.push_back('\t');
                }
            sJSON.push_back('}');
        }

        return sJSON;
    }

    static std::wstring WriteJSONByObject(sJSONObject *Object, bool Format = true, bool Brackets = true,
                                          size_t Level = 1) {
        std::wstring sJSON;
        if (Brackets) {
            sJSON = L"{";
            if (Format) {
                sJSON.push_back('\n');
            }
        }
        for (auto &Node: Object->Children) {
            if (Format)
                sJSONHelperCountDown(Level) {
                    sJSON.push_back('\t');
                }

            sJSON.append(L"\"" + Node.first + L"\"");
            sJSON.append(L":");
            sJSON.append(WriteValue(Node.second->Value, Format, Brackets, Level + 1));

            sJSON.push_back(',');
            if (Format) {
                sJSON.push_back('\n');
            }
        }

        sJSON.pop_back();
        if (Format) {
            sJSON.pop_back();
            sJSON.push_back('\n');
        }
        if (Brackets) {
            if (Format)
                sJSONHelperCountDown(Level - 1) {
                    sJSON.push_back('\t');
                }
            sJSON.push_back('}');
        }

        return sJSON;
    }

    static std::wstring
    WriteJSONByArray(sJSONArray *Object, bool Format = true, bool Brackets = true, size_t Level = 1) {
        std::wstring sJSON;
        if (Brackets) {
            sJSON = L"[";
            if (Format) {
                sJSON.push_back('\n');
            }
        }
        for (auto &Node: Object->ValueSet) {
            if (Format)
                sJSONHelperCountDown(Level) {
                    sJSON.push_back('\t');
                }

            sJSON.append(WriteValue(Node, Format, Brackets, Level));

            sJSON.push_back(',');
            if (Format) {
                sJSON.push_back('\n');
            }
        }

        sJSON.pop_back();
        if (Format) {
            sJSON.pop_back();
            sJSON.push_back('\n');
        }
        if (Brackets) {
            if (Format)
                sJSONHelperCountDown(Level - 1) {
                    sJSON.push_back('\t');
                }
            sJSON.push_back(']');
        }

        return sJSON;
    }

    static std::wstring WriteValue(sJSONValue *Node, bool Format = true, bool Brackets = true, size_t Level = 1) {
        if (Node->GetType() == sJSONValueType::Object) {
            return WriteJSONByObject(Node->To<sJSONObject *>(), Format, Brackets, Level + 1);
        }
        if (Node->GetType() == sJSONValueType::Array) {
            return WriteJSONByArray(Node->To<sJSONArray *>(), Format, Brackets, Level + 1);
        }
        if (Node->GetType() == sJSONValueType::Null) {
            return L"null";
        } else {
            if (sJSONRealValue<std::wstring>::Equal(Node)) {
                return L"\"" + ((sJSONRealValue<std::wstring> *) Node)->Value + L"\"";
            }
            if (sJSONRealValue<int>::Equal(Node)) {
                return std::to_wstring(((sJSONRealValue<int> *) Node)->Value);
            }
            if (sJSONRealValue<double>::Equal(Node)) {
                return std::to_wstring(((sJSONRealValue<double> *) Node)->Value);
            }
            if (sJSONRealValue<bool>::Equal(Node)) {
                if (((sJSONRealValue<bool> *) Node)->Value) {
                    return L"true";
                } else {
                    return L"false";
                }
            }
        }

        return L"?";
    }
};