/*
Name : Rohit Kumar
Roll Number : 2001CS55
Declaration of authorship : I hereby certify that the source code I am submitting is entirely my own original work except, where otherwise indicated.
*/

#include <bits/stdc++.h>
using namespace std;

struct codeline
{
    string linePc;
    string opcode;
    int optype; // tells whether the line has an operand, what type of operand - value/offset
    string value;
    string line;
};

struct mnemonics
{
    int opcode;
    int oneop;
};

// function definitions
void sepwords(string &s);
void zeroopcheck(string &s, string &ogline);
void oneopcheck(string &s, string &t, string &ogline);
void labelcheck(string &s);
bool validlabel(string &s);
bool decimalcheck(string &t);
bool octalcheck(string &t);
bool hexcheck(string &t);
void signextend(string &s, int x);
void converthex(string &s, int totalbits, int value);

// data structures
int pc, currline;
map<string, mnemonics> mnemo, labels;
vector<string> errors, warnings;
vector<codeline> lines;
vector<string> words;

int main(int argc, char *argv[])
{
    // first - opcode - -1 if no opcode
    // second - checker for type of operand - 0 if no operands, 1 if offset, 2 if value
    mnemo["data"] = {-1, 2}, mnemo["ldc"] = {0, 2}, mnemo["adc"] = {1, 2}, mnemo["ldl"] = {2, 2}, mnemo["stl"] = {3, 2}, mnemo["ldnl"] = {4, 2},
    mnemo["stnl"] = {5, 2}, mnemo["add"] = {6, 0}, mnemo["sub"] = {7, 0}, mnemo["shl"] = {8, 0}, mnemo["shr"] = {9, 0}, mnemo["adj"] = {10, 2},
    mnemo["a2sp"] = {11, 0}, mnemo["sp2a"] = {12, 0}, mnemo["call"] = {13, 1}, mnemo["return"] = {14, 0}, mnemo["brz"] = {15, 1},
    mnemo["brlz"] = {16, 1}, mnemo["br"] = {17, 1}, mnemo["HALT"] = {18, 0}, mnemo["SET"] = {-1, 2};

    if (argc != 2)
    {
        cout << "Invalid execution format. The command should be of the form - './asm filename.asm'.\n";
        return 0;
    }

    // opening the file
    string filename = argv[1];
    fstream asmcode;
    asmcode.open(filename, ios::in);
    if (!asmcode)
    {
        cout << "File does not exist.";
        return 0;
    }

    // scanning all the instructions one by one
    string s, t, ogline;
    int x;
    pc = 0, currline = 0;
    while (getline(asmcode, s))
    {
        ogline = s; // stores the original line
        words.clear();
        currline++;
        sepwords(s); // stores all the words in vector<string> words, detects labels and comments

        if ((int)words.size() == 1) // so the word is either a label, or a 0-operand mnemonic
        {
            s = words[0];
            x = (int)words[0].size();

            if (mnemo.count(s)) // checking for 0-operand mnemonic
                zeroopcheck(s, ogline);
            else if (words[0][x - 1] == ':') // checking for label
            {
                labelcheck(s);
                converthex(s, 8, pc);
                lines.push_back({s, "\t\t\t", -1, "", ogline});
                continue;
            }
            else
                errors.push_back("Invalid syntax at line number " + to_string(currline));
        }
        else if ((int)words.size() == 2) // either a label and a 0-operand mnemonic, or a 1-operand mnemonic
        {
            s = words[0], t = words[1];
            x = (int)words[0].size();
            if (words[0][x - 1] == ':') // checking for label
            {
                labelcheck(s);

                if (mnemo.count(t)) // if label is the first word, then the second/last word should be 0-operand mnemonic
                    zeroopcheck(t, ogline);
                else
                    errors.push_back("Invalid syntax at line number " + to_string(currline));
            }
            else if (mnemo.count(s)) // checking for 1-operand mnemonic
                oneopcheck(s, t, ogline);
            else
                errors.push_back("Invalid syntax at line number " + to_string(currline));
        }
        else if ((int)words.size() >= 3) // it can only be a label, followed by a 1-operand mnemonic
        {
            s = words[0];
            x = (int)words[0].size();
            if (words[0][x - 1] == ':') // checking for label
            {
                labelcheck(s);

                s = words[1], t = words[2];
                if (mnemo.count(s)) // checking for 1-operand mnemonic
                {
                    if ((int)words.size() == 3)
                        oneopcheck(s, t, ogline);
                    else
                        errors.push_back("Extra operand(s) at line number " + to_string(currline));
                }
                else
                    errors.push_back("Invalid syntax at line number " + to_string(currline));
            }
            else if (mnemo.count(s)) // if first word is mnemonic, then it definitely has extra operands, as there can be at most one operand to a mnemonic, but we have 2 words.
                errors.push_back("Extra operand(s) at line number " + to_string(currline));
            else
                errors.push_back("Invalid syntax at line number " + to_string(currline));
        }
        else if ((int)words.size() == 0) // empty line
            continue;
        pc++;
    }
    asmcode.close(); // close the asm file

    for (auto i : labels)
    {
        if (i.second.opcode == -1)
            errors.push_back("Undefined label '" + i.first + "'");
        if (i.second.oneop == false)
            warnings.push_back("Unused label '" + i.first + "'");
    }

    // creating the log file
    string logfile = filename.substr(0, filename.find(".")) + ".log";
    fstream log;
    log.open(logfile, ios::out);

    x = 1;
    for (auto i : errors)
    {
        log << x << ". "
            << "Error: " << i << "\n";
        x++;
    }

    for (auto i : warnings)
    {
        log << x << ". "
            << "Warning: " << i << "\n";
        x++;
    }

    log.close();
    if ((int)errors.size() > 0) // if there is any error, then create an empty listing file and close.
    {
        string listfile = filename.substr(0, filename.find(".")) + ".lst";
        fstream listing;
        listing.open(listfile, ios::out);
        listing.close();
        return 0;
    }

    // creating the listing file
    string listfile = filename.substr(0, filename.find(".")) + ".lst";
    fstream listing;
    listing.open(listfile, ios::out);
    for (auto i : lines)
    {
        if (i.optype >= 2) // operand is a label
        {
            if (i.optype == 2) // operand is a label and should be a value. So, take absolute
                x = labels[i.value].opcode;
            else if (i.optype == 3) // operand is a label and should be an offset. So, take displacement
                x = labels[i.value].opcode - stoi(i.linePc, nullptr, 16) - 1;
            converthex(s, 8 - (int)i.opcode.size(), x);
            i.value = s;
        }
        listing << i.linePc << " " << i.value << i.opcode << "\t" << i.line << "\n";
    }
    listing.close();

    // creating object file
    listing.open(listfile, ios::in);
    string objectfile = filename.substr(0, filename.find(".")) + ".o";
    fstream machinecode;
    machinecode.open(objectfile, ios_base::out | ios_base::binary);
    vector<unsigned long> objfile;
    while (getline(listing, s))
    {
        words.clear();
        sepwords(s);
        if ((int)words.size() == 2 && labels.count(words[1].substr(0, (int)words[1].size() - 1))) // ex. 00000002 label:
            continue;                                                                             // if the line contains only label, continue without adding to the object file
        objfile.push_back(stoul(words[1], nullptr, 16));                                          // push the unsigned integer values to a vector
    }
    machinecode.write((char *)(&objfile[0]), objfile.size() * sizeof(unsigned long)); // writing the values in binary
    listing.close();
    machinecode.close();
    return 0;
}

void sepwords(string &s)
{
    string word, tmp;
    if (s.find(';') != -1) // anything following ';' is ignored
        s = s.substr(0, s.find(';'));
    if (s.empty())
        return;
    stringstream ss(s); // creating a stream of the string, so we can take the input word by word
    while (ss >> word)
    {
        if (word.find(':') != -1) // if there is a label in the word, separate it
        {
            tmp = word.substr(0, word.find(':') + 1);
            if (!tmp.empty())
                words.push_back(tmp); // separating label
            if (word.find(':') != ((int)word.size() - 1))
                word = word.substr(word.find(':') + 1, word.size()); // if there is any word after label, add it
            else
                word = "";
        }
        if (!word.empty())
            words.push_back(word);
    }
}

void zeroopcheck(string &s, string &ogline)
{
    if (mnemo[s].oneop > 0)
        errors.push_back("Missing operand at line number " + to_string(currline));
    else
    {
        string a, b, c;

        converthex(a, 8, pc);
        converthex(b, 2, mnemo[s].opcode);
        converthex(c, 6, 0); // value is 0, since there is no operand

        lines.push_back({a, b, 0, c, ogline}); // the 0 in the 3rd arguement tells about the type of operand
    }
}

void oneopcheck(string &s, string &t, string &ogline)
{
    if (mnemo[s].oneop == 0)
        errors.push_back("Extra operand at line number " + to_string(currline));
    else
    {
        int x = 0;
        if (decimalcheck(t))
            x = stoi(t, nullptr, 10);
        else if (hexcheck(t))
            x = stoi(t, nullptr, 16);
        else if (octalcheck(t))
            x = stoi(t, nullptr, 8);
        else if (validlabel(t)) // if label, check if it already has an address or not
        {
            if (labels.count(t))
            {
                x = labels[t].opcode;
                labels[t].oneop = true;
            }
            else
                labels[t] = {-1, true};
        }
        else
            errors.push_back("Invalid operand value at line number " + to_string(currline));

        string a, b, c;

        converthex(a, 8, pc);
        if (mnemo[s].opcode != -1)
        {
            converthex(b, 2, mnemo[s].opcode);
            converthex(c, 6, x);
        }
        else
        {
            b = "";
            converthex(c, 8, x); // if no opcode, provide all 8 bits to operand
        }
        if (labels.count(t))
            c = t; // pass label as it is instead of its value. Assign value later, as some labels will not have an address until all instructions are scanned
        if ((int)words.size() == 3 && s == "SET" && labels.count(words[0].substr(0, (int)words[0].size() - 1)))
            labels[words[0].substr(0, (int)words[0].size() - 1)].opcode = x; // set the value of label at the start, as the value given

        lines.push_back({a, b, 1 + (bool)(labels.count(t)) + (bool)(mnemo[s].oneop == 1 && labels.count(t)), c, ogline});
    }
}

void labelcheck(string &s)
{
    s.pop_back();
    if (labels.count(s))
    {
        if (labels[s].opcode != -1)
            errors.push_back("Label repeated at line number " + to_string(currline));
        else
            labels[s] = {pc, true};
    }
    else
    {
        if (validlabel(s))
            labels[s] = {pc, false};
        else
            errors.push_back("Invalid label '" + s + "' at line number " + to_string(currline));
    }
}

bool validlabel(string &s)
{
    if ((s[0] >= 'A' && s[0] <= 'Z') || (s[0] >= 'a' && s[0] <= 'z'))
    {
        for (auto i : s)
        {
            if ((i >= 'A' && i <= 'Z') || (i >= 'a' && i <= 'z') || (i >= '0' && i <= '9'))
                continue;
            else
                return false;
        }
        return true;
    }
    else
        return false;
}

bool decimalcheck(string &t)
{
    string tmp = t;
    if (t[0] == '-' || t[0] == '+')
        tmp = t.substr(1, t.size());
    if (tmp[0] == '0' && (int)tmp.size() > 1)
        return false;
    for (auto i : tmp)
        if (i < '0' || i > '9')
            return false;
    return true;
}

bool octalcheck(string &t)
{
    if (t[0] != '0')
        return false;
    int n = (int)t.size();
    t = t.substr(1, n);
    for (auto i : t)
        if (i < '0' || i > '7')
            return false;
    return true;
}

bool hexcheck(string &t)
{
    if ((int)t.size() <= 2 || t[0] != '0' || t[1] != 'x')
        return false;
    int n = (int)t.size();
    t = t.substr(2, n);
    for (auto i : t)
    {
        if ((i >= 'A' && i <= 'F') || (i >= 'a' && i <= 'f') || (i >= '0' && i <= '9'))
            continue;
        else
            return false;
    }
    return true;
}

void signextend(string &s, int x)
{
    int n = (int)s.size();
    string t = "";
    if (n > x)
    {
        s = s.substr(n - x, x);
        return;
    }
    for (int i = n; i < x; i++)
        t += "0";
    s = t + s;
}

void converthex(string &s, int totalbits, int value)
{
    stringstream ss;
    ss << hex << value;
    s = ss.str();
    signextend(s, totalbits);
}