#include "stdafx.h"
#include "Lexical.h"


bool Symbol::find_id(string id) {
    return se.find(id) != se.end();
}

void Symbol::push_id(string id) {
    se.insert(id);
}

bool Symbol::push(string id, int row, int col) {
    try
    {
        pair<int, int>  p(row, col);
        pair<string, pair<int, int>> q(id, p);
        table.insert(q);
    }
    catch (const std::exception&)
    {
        return false;
    }

    return true;
}

bool Symbol::find(string id) {
    for (auto v : table) {
        if (v.first == id)return true;
    }
    return false;
}


Lexical::Lexical() {
    instring.push_back("");
    row = col = 0;
}

Lexical::Lexical(string str) {
    int len = (int)str.length();
    string s = "";
    for (int i = 0; i < len; ++i) {
        if (str[i] == '\n') {
            if (s.length() > 0) instring.push_back(s);
            s = "";
        }
        else {
            s += str[i];
        }
    }
    if (s != "") {
        instring.push_back(s);
    }
    row = col = 0;
}

bool Lexical::isSpcae(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

bool Lexical::isId(string id) {
    if (isdigit(id[0]) || id[0] == '_')return false;
    for (auto c : id) {
        if (!isalnum(c) && c != '_')return false;
    }
    return true;
}

bool Lexical::isKey(string k) {
    for (auto s : Key) {
        if (s == k)return true;
    }
    return false;
}
//验证单个字符的运算符
bool Lexical::isOperator(char ch) {
    for (auto s : Operator) {
        if (s[0] == ch)return true;
    }
    return false;
}
//验证多个字符的运算符
bool Lexical::isOperator(string ch) {
    for (auto s : Operator) {
        if (s == ch)return true;
    }
    return false;
}

bool Lexical::isDelimitor(char li) {
    for (auto c : Delimitor)
        if (c[0] == li) return true;
    return false;
}


string Lexical::to_json_str(int type, string val) {
    int len = (int)val.length();
    string position = ")\",\"position\":\"(" + to_string(row + 1) + ", " + to_string(col + 1 - len) + ")\"";
    string tuple = "\",\"tuple\":\"(";

    switch (type)
    {
    case 0:
        return "{\"key\":\"" + val + tuple + to_string(1) + ", " + val + position + "},";
    case 1:
        //若标识符不存在则推进数组，否则记录其位置
        if (sy.find_id(val)) { sy.push(val, row, col); }
        else {
            sy.push_id(val);
        }
        return "{\"identifier\":\"" + val + tuple + to_string(6) + ", " + val + position + "},";
    case 2:
        if (sy.find_id(val)) { sy.push(val, row, col); }
        else {
            sy.push_id(val);
        }
        return "{\"number\":\"" + val + tuple + to_string(5) + ", " + val + position + "},";
    case 3:
        return "{\"comment\":\"" + val + tuple + to_string(7) + ", " + val + position + "},";
    case 4:
        return "{\"operator\":\"" + val + tuple + to_string(3) + ", " + val + position + "},";
    case 5:
        return "{\"delimiter\":\"" + val + tuple + to_string(2) + ", " + val + position + "},";
    case 6:
        return "{\"string\":\"" + val + tuple + to_string(8) + ", " + val + position + "},";
    case 7:
        return "{\"error\":\"" + val + "\",\"tuple\":\"" + "ERROR" + position.substr(1) + "},";
    default:
        return "{}";
    }

}
string Lexical::run() {
    string result = "[";
    for (row = 0; row < (int)instring.size(); ++row) {
        int len = (int)instring[row].length();
        string s = instring[row];
        char ch = NULL;
        for (col = 0; col < len; ++col) {
            ch = s[col];
            //解析标识符或者关键字，字母开头后接字母或者数字或者下划线
            if (isalpha(ch)) {
                string key(1, ch
                );
                while (col + 1 < len && (isalnum(s[col + 1]) || s[col + 1] == '_')) {
                    col = col + 1;
                    ch = s[col];
                    key += ch;
                }
                if (isKey(key)) {
                    result += to_json_str(0, key);
                }
                else if (isId(key)) {
                    result += to_json_str(1, key);
                }
                else {
                    result += to_json_str(7, key);
                }
                //cout << key << "\n";
            }
            else if (ch == '0' && ((s[col + 1] == 'x' || s[col + 1] == 'X') || (s[col + 1] == 'o' || s[col + 1] == 'O'))) {
                string number(1, ch);
                number += s[col + 1];
                col += 2;
                ch = s[col];
                while (col < len && ((ch >= 'A'&&ch <= 'F' || ch >= 'a'&&ch <= 'f') || isdigit(ch))) {
                    number += ch;
                    col += 1;
                    ch = s[col];
                }
                //回溯一个字符
                col -= 1;
                result += to_json_str(2, number);
            }
            else if (isdigit(ch)) {
                string number(1, ch);
                bool error = false;
                while (col + 1 < len && ((isdigit(s[col + 1]) || s[col + 1] == 'e' || s[col + 1] == 'E' || s[col + 1] == '.'))) {
                    col = col + 1;
                    ch = s[col];
                    number += ch;
                    if ((ch == 'e' || ch == 'E')) {
                        if ((s[col + 1] == '+' || s[col + 1] == '-') && col + 2 < len&&isdigit(s[col + 2])) {
                            number += s[col + 1];
                            col += 1;
                            while (col + 1 < len&&isdigit(s[col + 1])) {
                                col = col + 1;
                                ch = s[col];
                                number += ch;
                                continue;
                            }
                        }
                        else {
                            error = true;
                        }

                    }
                    if (ch == '.') {
                        if (isdigit(s[col + 1])) {
                            continue;
                        }
                        else {
                            error = true;
                            break;
                        }
                    }
                }
                result += error ? to_json_str(7, number) : to_json_str(2, number);
            }
            else if (ch == '/' && col + 1 < len && s[col + 1] == '/') {
                string comment = s.substr(col + 2, len);
                col = len;
                result += to_json_str(3, comment);
            }
            else if (ch == '/' && col + 1 < len && s[col + 1] == '*') {
                bool error = false;
                col += 2;
                ch = s[col];
                string comment(1, ch);
                if (col >= len) {
                    col = -1;
                    ch = s[0];
                    row += 1;
                    len = (int)s.length();
                    s = instring[row];
                }
                while (col + 1 < len && (ch != '*'&&s[col + 1] != '/')) {
                    col += 1;
                    ch = s[col];
                    comment += ch;
                    if (col >= len - 1 && row + 1 < (int)instring.size()) {
                        row += 1;
                        s = instring[row];
                        col = 0;
                        ch = s[col];
                        len = (int)s.length();
                        comment += ch;
                        while (len == 1 && row + 1 < (int)instring.size()) {
                            row += 1;
                            s = instring[row];
                            col = 0;
                            ch = s[col];
                            len = (int)s.length();
                            comment += ch;
                        };
                    }
                    if (col == len - 1 && row == (int)instring.size() - 1) {
                        error = true;
                    }
                }
                col += 1;
                result += to_json_str(error ? 7 : 3, error ? comment : comment.substr(0, comment.length() - 1));
            }
            else if (isOperator(ch)) {
                string op(1, ch);
                //双字符运算符，但是匹配失败 , 运算符的终结符是数字字母和空格
                while (col + 1 < len && !isalnum(s[col + 1]) && !isspace(s[col + 1])) {
                    col += 1;
                    ch = s[col];
                    op += ch;
                }
                if (isOperator(op)) {
                    result += to_json_str(4, op);
                }
                else if (op.length() >= 2) {
                    //回溯
                    col -= op.length() - 1;
                    result += to_json_str(4, string(1, op[0]));
                }
                else {
                    result += to_json_str(7, op);
                }

                //cout << op << "\n";
            }
            else if (isDelimitor(ch)) {
                string op(1, ch);
                result += to_json_str(5, op);
            }

            else if (ch == '\'' || ch == '"') {
                bool issingle_quote = ch == '\'';
                bool error = false;
                string str = "";
                if (issingle_quote) {
                    while (col + 1 < len&&s[col + 1] != '\'') {
                        col += 1;
                        ch = s[col];
                        str += ch;
                    }
                    //引号未闭合error，否则跳到下一个字符
                    s[col + 1] != '\'' ? error = true : col += 1;
                }
                else {
                    while (col + 1 < len&&s[col + 1] != '"') {
                        col += 1;
                        ch = s[col];
                        str += ch;
                    }
                    s[col + 1] != '"' ? error = true : col += 1;
                }
                result += to_json_str(error ? 7 : 6, str);

            }
            else if (!isSpcae(ch)) {
                string err(1, ch);
                while (col + 1 < len && !isSpcae(s[col + 1]) && !isDelimitor(s[col + 1])) {
                    col += 1;
                    ch = s[col];
                    err += ch;
                }
                result += to_json_str(7, err);
                //cout << err << "\n";
            }
            //cout << ch << '\t' << row << " " << col << "\n";
        }
    }

    return result.substr(0, result.length() - 1) + "]";
}