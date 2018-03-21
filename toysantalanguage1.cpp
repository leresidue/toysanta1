// toysantalanguage1.cpp : définit le point d'entrée pour l'application console.
// © 2018 Frédérique Brisson-Lambert

#include "stdafx.h"


using namespace std;


int main()
{
	
	

	wstring		fname;
	
	wcout << L"Filename: ";
	wcin >> fname;

	wifstream	file(fname);
	
	wstring		toy;
	toyMACHINE	mac;
	toyMAKER	tm;
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);

	if(file.is_open()) {
		file.imbue(std::locale(file.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
		file.get();
		
		wcout.imbue(std::locale(wcout.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
		wcin.imbue(std::locale(wcin.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
		while(getline(file, toy)) {
			//wcout << toy << L"\n";
			int err = tm.make(toy, &mac);
			if(err < -10) {
				wcout  << err << L"\n";
				break;
			} else if(err >=0) {
				//wcout << err << endl;
				//tm.target->execline(err);
			}
			//wcout << toy << endl;
		}
		
		file.close();
		tm.exec(tm.exec_pos);
	}
	
	while(getline(wcin, toy)) {
		int	err = tm.make(toy, &mac);
		if(err < 0 && err != -8) {
			if(err == -3) {
				for(int i = 0; i < tm.code_subs.size(); i++) {
					wcout << L"    ";
				}
			} else {
				wcout << err << L"\n";
			}
		} else {
			tm.target->execline(err);
		}
	}

    return 0;
}

