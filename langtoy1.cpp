// langtoy1.cpp : définit le point d'entrée de l'application.
//

#include "pch.h"
#include <iostream>

#include <cstdint>

#include <cmath>

#include <vector>
#include <unordered_map>
#include <sstream>
#include <string>
#include <fstream>
#include <locale>
#include <codecvt>

#include <iomanip>
#include <clocale>
#include <cstring>
#include <cwchar>
#include <cuchar>

#include <cstdlib>

#include "ts10000.h"
#include "tsF0000.h"

using namespace std;

class contextlt {
public:
	std::vector<std::wstring>	params;
	wchar_t		*data;
	size_t		dlen, dmax;

	bool setdmax(size_t pmax) {
		wchar_t	*data2 = nullptr;
		if(pmax == 0) {
			free(data);
			dmax = 0;
			return true;
		}
		if(data) {
			data2 = (wchar_t*)realloc(data, pmax*sizeof(wchar_t));
		} else {
			data2 = (wchar_t*)malloc(pmax*sizeof(wchar_t));
		}
		if(data2 != nullptr) {
			data = data2;
			dmax = pmax;
			return true;
		}
		return false;
	}
	contextlt(const char *pname) {
		data = nullptr;
		dlen = dmax = 0;

		FILE	*fp = nullptr;
		errno_t	err;
		err = fopen_s(&fp, pname, "rb");


		if(err == 0 && fp) {
			int	sizf = 0, sizfre;
			if(fseek(fp, 0, SEEK_END)==0) {
				if(sizf = ftell(fp)) {
					if(fseek(fp, 0, SEEK_SET)==0) {
						if(setdmax(sizf)) {
							char	*ncda, *releaseda;
							if(dmax > 0) {
								releaseda = ncda = (char*)malloc(sizf);
								sizfre = fread(ncda, sizeof(char), sizf, fp);
								if(sizfre == sizf) {
									std::setlocale(LC_ALL, "en_US.utf8");
									std::mbstate_t state{};
									const char *ncdatend = &ncda[0] + sizfre;
									int	skipbom = 0;
									if(sizf >= 3 && ncda[0] == (char)0xef && ncda[1] == (char)0xbb && ncda[2] == (char)0xbf) {
										skipbom = 3;
										ncda += 3;
									}
									for(int i = skipbom; i < dmax; i++) {
										int	ret;
										
										ret = std::mbrtoc16((char16_t*)&data[i-skipbom], ncda, ncdatend-ncda+1, &state);
										if(ret <= 0) break;
										ncda += ret;
										//wcout << data[i-skipbom];
										
									}
									dlen = dmax;
								} else {
									setdmax(0);
									wcout << L"ERR" << __LINE__ << L" ";
								}
								free(releaseda);
							} else {
								wcout << L"ERR" << __LINE__ << L" ";
							}
						} else {
							wcout << L"ERR" << __LINE__ << L" ";
						}
					}
				}
			}

			fclose(fp);
		}
	}
	contextlt() {
		data = nullptr;
		dlen = dmax = 0;
	}
};

bool testextension(const char *str, const char *ext) {
	size_t	strsz, extsz;
	strsz = strlen(str);
	extsz = strlen(ext);
	if(strsz >= extsz) {
		for(uint64_t i = 0; i < extsz; i++) {
			if(str[strsz-(i+1)] != ext[extsz-(i+1)]) {
				return false;
			}
		}
		return true;
	}
	return false;
}

int main(int argc, char *argv[])
{
	std::vector<std::wstring>	params;
	std::vector<contextlt*>		list;
	contextlt	*targ = 0;

	for(int i = 1; i < argc; i++) {
		if(argv[i]) {
			if((argv[i])[0] == '/' || (argv[i])[0] == '-') {
				size_t	sz, retval;
				wchar_t	*wc;
				sz = strlen(argv[i]);
				wc = new wchar_t(sz+1);
				mbstowcs_s(&retval, wc, sz+1, argv[i], sz);
				if(targ) {
					targ->params.push_back(wc);
				} else {
					params.push_back(wc);
				}
				delete wc;
			} else if(testextension(argv[i], ".toy")) {
				targ = new contextlt(argv[i]);
				wcout << L"has read " << argv[i] << endl;
				list.push_back(targ);
			} else {
				wcout << L"ERR" << __LINE__ << L" ";
			}
		}
	}
	toyMACHINE	mac;
	toyMAKER	tm;
	int		reeo = 0;
	for(auto i = list.begin(); i != list.end(); i++) {
		wchar_t	*pl;
		std::wstring	dogo;
		pl = (*i)->data;
		for(int u = 0; u < (*i)->dlen; u++) {
			dogo += (*i)->data[u];
			if((*i)->data[u] == L'\n') {
				wcout << dogo;
				int err = tm.make(dogo,&mac);
				if(err < -10) {
					std::wcout  << err << L"\n";
					reeo = 1;
					break;
				} else if(err >=0) {
					//wcout << err << endl;
					//tm.target->execline(err);
				}
				dogo = L"";
			}
		}
	}
	if(reeo == 0) {
		tm.exec(tm.exec_pos);
		wstring	dod;
		while(getline(std::wcin,dod)) {
			int	err = tm.make(dod,&mac);
			if(err < 0 && err != -8) {
				if(err == -3 || err == -2) {

				} else {
					wcout << err << L"\n";
				}
			} else {
				tm.target->execline(err);
			}
			if(err == -8 || err == -3 || err == -2) {
				for(int i = 0; i < tm.code_subs.size(); i++) {
					wcout << L"    ";
				}
			}
		}
	}
	return 0;
}
