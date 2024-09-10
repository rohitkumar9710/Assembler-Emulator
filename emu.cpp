#include <bits/stdc++.h>
using namespace std;
#define maxsize 1000000
void signextend(string &s, int x);
void converthex(string &s, int totalbits, int value);
int opcode, val, pc = 0, sp = 0, rega = 0, regb = 0;
vector<int> memory(1000000);

int main(int argc, char *argv[])
{
	// execution format: ./emu filename.o [options]
	if (argc != 3)
	{
		cout << "Invalid command. Correct execution format: ./emu filename.o [options]\n"
			 << "-isa    display ISA\n"
			 << "-wipe   wipe written flags before execution\n"
			 << "-before show memory dump before execution\n"
			 << "-trace  show instruction trace\n"
			 << "-read   show memory reads\n"
			 << "-write  show memory writes\n"
			 << "-after  show memory dump after execution\n";
		return 0;
	}
	string filename = argv[1];
	fstream objstream;
	objstream.open(filename, ios_base::in | ios_base::binary);
	if (!objstream)
	{
		cout << "Invalid file.\n";
		return 0;
	}
	// getting the size of the file
	objstream.seekg(0, objstream.end);
	int n = objstream.tellg();
	objstream.seekg(0, objstream.beg);
	// In a binary file, 1 character has 1 byte of data. We want to read one interger (4 bytes) at a time
	objstream.read((char *)&memory[0], n);
	objstream.close();
	map<string, pair<int, int>> mnemo;
	map<int, string> m;
	mnemo["data"] = {-1, 2}, mnemo["ldc"] = {0, 2}, mnemo["adc"] = {1, 2}, mnemo["ldl"] = {2, 2}, mnemo["stl"] = {3, 2}, mnemo["ldnl"] = {4, 2},
	mnemo["stnl"] = {5, 2}, mnemo["add"] = {6, 0}, mnemo["sub"] = {7, 0}, mnemo["shl"] = {8, 0}, mnemo["shr"] = {9, 0}, mnemo["adj"] = {10, 2},
	mnemo["a2sp"] = {11, 0}, mnemo["sp2a"] = {12, 0}, mnemo["call"] = {13, 1}, mnemo["return"] = {14, 0}, mnemo["brz"] = {15, 1},
	mnemo["brlz"] = {16, 1}, mnemo["br"] = {17, 1}, mnemo["HALT"] = {18, 0}, mnemo["SET"] = {-1, 2};
	for (auto i : mnemo)
		m[i.second.first] = i.first;
	string s, t, option = argv[2];
	if (option == "-isa")
	{
		cout << "\nOpcode\t\tMnemonic\tOperand\n\n";
		for (auto i : m)
		{
			if (i.first != -1)
				converthex(s, 8, i.first);
			else
				continue;
			cout
				<< s << "\t" << i.second << "\t\t";
			if (mnemo[i.second].second == 1)
				cout << "offset\n";
			else if (mnemo[i.second].second == 2)
				cout << "value\n";
			else
				cout << "--\n";
		}
		cout << "\t\tSET\t\tvalue\n";
		cout << "\t\tdata\t\tvalue\n\n";
	}
	if (option == "-wipe")
	{
		cout << "All flags have been reset.\n";
		pc = 0, sp = 0, rega = 0, regb = 0;
	}
	if (option == "-before")
	{
		cout << "\nMemory dump before execution:\n\n";

		for (int i = 0; i < 10; i++)
		{
			converthex(s, 8, 4 * i);
			cout << "Memory[" << s << "]: ";
			for (int j = 0; j < 4; j++)
			{
				converthex(s, 8, memory[4 * i + j]);
				cout << s << "\t";
			}
			cout << "\n";
		}
		cout << "\n";
	}
	int num = 0;
	for (pc = 0; pc < (n / 4); pc++)
	{
		num++;
		converthex(s, 8, memory[pc]);
		opcode = stoi(s.substr(6, 2), nullptr, 16);
		t = s.substr(0, 6);
		if (t[0] > '7')
			t = "ff" + t;
		else
			t = "00" + t;
		val = stoul(t, nullptr, 16);
		// cout << pc << " ";
		if (option == "-trace" && opcode >= 0 && opcode <= 18)
		{
			string p, a, b, stackp;
			converthex(p, 8, pc);
			converthex(a, 8, rega);
			converthex(b, 8, regb);
			converthex(stackp, 9, sp);
			cout << "PC = " << p << ", SP = " << stackp << ", Reg-A = " << a << ", Reg-B = " << b << ", " << m[opcode] << " " << t << "\n";
		}

		if (opcode == 0)
		{
			regb = rega;
			rega = val;
		}
		else if (opcode == 1)
		{
			rega += val;
		}
		else if (opcode == 2)
		{
			regb = rega;
			int x = sp + val;
			if (x < 0 || x >= maxsize)
			{
				if (option == "-trace")
					cout << "Memory address out of bounds\n";
				break;
			}
			rega = memory[x];
			if (option == "-read")
			{
				converthex(s, 8, x);
				converthex(t, 8, rega);
				cout << "Reading from Memory[" << s << "] = " << t << "\n";
			}
		}
		else if (opcode == 3)
		{
			int x = sp + val;
			if (x < 0 || x >= maxsize)
			{
				if (option == "-trace")
					cout << "Memory address out of bounds\n";
				break;
			}
			if (option == "-write")
			{
				converthex(s, 8, x);
				cout << "Writing into Memory[" << s << "]\n";
				converthex(s, 8, memory[x]);
				memory[x] = rega;
				rega = regb;
				converthex(t, 8, rega);
				cout << "Previous value = " << s << "\nNew value = " << t << "\n\n";
			}
			memory[x] = rega;
			rega = regb;
		}
		else if (opcode == 4)
		{
			int x = rega + val;
			if (x < 0 || x >= maxsize)
			{
				if (option == "-trace")
					cout << "Memory address out of bounds\n";
				break;
			}
			rega = memory[x];
			if (option == "-read")
			{
				converthex(s, 8, x);
				converthex(t, 8, rega);
				cout << "Reading from Memory[" << s << "] = " << t << "\n";
			}
		}
		else if (opcode == 5)
		{
			int x = rega + val;
			if (x < 0 || x >= maxsize)
			{
				if (option == "-trace")
					cout << "Memory address out of bounds\n";
				break;
			}
			memory[x] = regb;
			if (option == "-write")
			{
				converthex(s, 8, x);
				cout << "Writing into Memory[" << s << "]\n";
				converthex(s, 8, memory[x]);
				memory[x] = regb;
				converthex(t, 8, regb);
				cout << "Previous value = " << s << "\nNew value = " << t << "\n\n";
			}
		}
		else if (opcode == 6)
		{
			rega += regb;
		}
		else if (opcode == 7)
		{
			rega = regb - rega;
		}
		else if (opcode == 8)
		{
			rega = regb << rega;
		}
		else if (opcode == 9)
		{
			rega = regb >> rega;
		}
		else if (opcode == 10)
		{
			sp += val;
			if (sp > 1000000)
			{
				if (option == "-trace")
					cout << "Stack memory address out of bounds\n";
				break;
			}
		}
		else if (opcode == 11)
		{
			sp = rega;
			if (sp > 1000000)
			{
				if (option == "-trace")
					cout << "Stack memory address out of bounds\n";
				break;
			}
			rega = regb;
		}
		else if (opcode == 12)
		{
			regb = rega;
			rega = sp;
		}
		else if (opcode == 13)
		{
			regb = rega;
			rega = pc;
			pc += val;
		}
		else if (opcode == 14)
		{
			pc = rega;
			rega = regb;
		}
		else if (opcode == 15)
		{
			if (rega == 0)
				pc += val;
		}
		else if (opcode == 16)
		{
			if (rega < 0)
				pc += val;
		}
		else if (opcode == 17)
		{
			pc += val;
		}
		else if (opcode == 18)
		{
			if (option == "-trace")
				cout << "Executed finished.\n";
			break;
		}
		if (pc < 0 || pc >= (n / 4))
		{
			if (option == "-trace")
				cout << "PC out of bounds.\n";
			return 0;
		}
	}
	if (option == "-after")
	{
		cout << "Memory dump after execution:\n";
		for (int i = 0; i < 10; i++)
		{
			converthex(s, 8, 4 * i);
			cout << s << " : ";
			for (int j = 0; j < 4; j++)
			{
				converthex(s, 8, memory[4 * i + j]);
				cout << s << " ";
			}
			cout << "\n";
		}
	}
	cout << num << " instructions executed.\n";
	return 0;
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