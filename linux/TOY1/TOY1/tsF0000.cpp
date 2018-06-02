/********************************************************************
	Copyright (C) 2018 Frédérique Brisson-Lambert

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>
**************************************************************************/

#include "stdafx.h"


INPUTtoy::INPUTtoy() {
	name = L"input";
	auto_exec = true;
}

int INPUTtoy::func(toyCONTEXT *ctx, int it, size_t csz) {
	std::wstring	*str;
	std::wstring	rere;
	toyFUNC			*conv = nullptr;
	it++;
	csz--;
	uint32_t	msg = ctx->code[it] & tsbcmsg;
	uint32_t	bco = ctx->code[it] &~tsbcmsg;
	std::wstring	*dstr = nullptr;
	int64_t		*d64 = nullptr;
	double		*f64 = nullptr;

	msg >>= 32UL-4UL;
	if(msg == ts_cash) {
		d64 = ctx->getdollar();
	} else if(msg == ts_num || msg == ts_pnum) {
		d64 = ctx->findnum(ctx->code[it], (tsbtype)msg);
	} else if(msg == ts_dec || msg == ts_pdec) {
		f64 = ctx->finddec(ctx->code[it], (tsbtype)msg);
	} else if(msg == ts_txt || msg == ts_ptxt) {
		dstr = ctx->findstr(ctx->code[it], it, (tsbtype)msg);
	}

	for(int i = it+1; i < it+csz; i++) {
			msg = ctx->code[i] & tsbcmsg;
			bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} else {
			bool	cancon = false;
			if(conv) {
				if(conv->okconvert(ctx->code[i])) {
					cancon = true;
					rere += conv->convert(ctx, ctx->code[i], it);
				}
			}
			if(cancon == false) {
				str = ctx->findstr(ctx->code[i], it);
				if(str) {
					rere +=  *str;
				}
			}
			conv = nullptr;
		}
	}
	if(dstr) {
		std::wstring	dstru;
		std::wcout << rere;
		std::wcin >> dstru;
		//std::getline(std::wcin, dstru);
		*dstr = dstru;
		//std::getline(std::wcin, (*dstr));
	}
	return __LINE__;
}

CONVERTtoy::CONVERTtoy() {
	name = L"convert";
	auto_exec = true;
}

int CONVERTtoy::func(toyCONTEXT *ctx, int it, size_t csz) {
	std::wstring	*str;
	std::wstring	rere;
	toyFUNC			*conv = nullptr;
	it++;
	csz--;
	uint32_t	msg = ctx->code[it] & tsbcmsg;
	uint32_t	bco = ctx->code[it] &~tsbcmsg;
	std::wstring	*dstr = nullptr;
	int64_t		*d64 = nullptr;
	double		*f64 = nullptr;

	msg >>= 32UL-4UL;
	if(ctx->code[it] == mktsbc(0, ts_cash)) {
		d64 = ctx->getdollar();
	} else if(msg == ts_num || msg == ts_pnum) {
		d64 = ctx->findnum(ctx->code[it], (tsbtype)msg);
	} else if(msg == ts_dec || msg == ts_pdec) {
		f64 = ctx->finddec(ctx->code[it], (tsbtype)msg);
	} else if(msg == ts_txt || msg == ts_ptxt) {
		dstr = ctx->findstr(ctx->code[it], (tsbtype)msg);
	}

	for(int i = it+1; i < it+csz; i++) {
			msg = ctx->code[i] & tsbcmsg;
			bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} else {
			bool	cancon = false;
			if(conv) {
				if(conv->okconvert(ctx->code[i])) {
					cancon = true;
					rere += conv->convert(ctx, ctx->code[i], it-1);
				}
			}
			if(cancon == false) {
				str = ctx->findstr(ctx->code[i], it-1);
				if(str) {
					rere +=  *str;
				}
			}
			conv = nullptr;
		}
	}
	if(d64) {
		*d64 = std::stoll(rere);
	}
	if(f64) {
		*f64 = std::stod(rere);
	}
	if(dstr) {
		*dstr = rere;
	}
	return __LINE__;
}


INTEGERtoy::INTEGERtoy(const wchar_t *pn) {
	name = pn;
	can_var = true;
	just_auto = true;
	if(name == L"decimal") {
		action = a_decimal;
	}
}
tsbcode INTEGERtoy::do_just(toyCONTEXT *ctx, int it, size_t csz) {
	std::wstring	vnam;
	toyFUNC			*conv = nullptr;
	for(int i = it; i < it+csz; i++) {
		uint32_t	msg = ctx->code[i] & tsbcmsg;
		uint32_t	bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} if(msg == tsbtype::ts_atxt) {
			std::wstring	*str;
			str = ctx->findstr(ctx->code[i], it);
			if(str) {
				vnam = *str;
				if(vnam.size()>2) {
					std::wstring	cvnam;
					tsbtype				prudence = tsbtype::ts_pnum;
					if(vnam[0] == L'[') prudence = tsbtype::ts_num;
					cvnam.assign(vnam.c_str()+1, vnam.size()-2);
					tsbcode nc = ctx->allocnum(0, cvnam, prudence);
					if(nc != invalidtsb) {
						uint32_t	s;
						msg = nc & tsbcmsg;
						bco = nc &~tsbcmsg;
						msg >>= 32UL-4UL;
							
						if(prudence == ts_pnum) {
							s = ctx->heap.back()->pnum.vars.size();
							ctx->code[i] = mktsbc(bco, ts_protect);
						} else {
							s = ctx->heap.back()->num.vars.size();
							ctx->code[i] = mktsbc(bco, ts_private);
						}
							
						ctx->heap.back()->offsets[prudence] = s;
						return invalidtsb;
					}
				}
			}
		}
	}
	return invalidtsb;
}
int INTEGERtoy::dfunc(toyCONTEXT *ctx, const tsbcode *code, size_t csz, int it) {
	std::wstring	vnam;
	double		data = 0;
	double			*num;
	toyFUNC			*conv = nullptr;
	for(int i = 0; i < csz; i++) {
		uint32_t	msg = code[i] & tsbcmsg;
		uint32_t	bco = code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(code[i]);
		} if(msg == tsbtype::ts_atxt && i == 0) {
			std::wstring	*str;
			str = ctx->findstr(code[i], it);
			if(str) vnam = *str;
		} else {
			num = ctx->finddec(code[i]);
			if(num && i > 0) {
				data += *num;
			} else if(i > 0) {
				int64_t	*inu;
				inu = ctx->findnum(code[i]);
				if(inu) {
					data += *inu;
				}
			}
			conv = nullptr;
		}
	}
	if(vnam.size()>2) {
		std::wstring	cvnam;
		tsbtype				prudence = tsbtype::ts_pdec;
		if(vnam[0] == L'[') prudence = tsbtype::ts_dec;
		cvnam.assign(vnam.c_str()+1, vnam.size()-2);
		ctx->allocdec(data, cvnam, prudence);
		return __LINE__;
	}
	return -__LINE__;
}
int INTEGERtoy::func(toyCONTEXT *ctx, int it, size_t csz) {
	if(action == a_decimal) return -__LINE__;
	std::wstring	vnam;
	int64_t			*data = nullptr;
	int64_t			*num;
	toyFUNC			*conv = nullptr;
	for(int i = it; i < it+csz; i++) {
		uint32_t	msg = ctx->code[i] & tsbcmsg;
		uint32_t	bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} else if(data == nullptr) {
			if(msg == ts_protect) {
				data = &(ctx->heap.back()->pnum.vars[bco]);
				ctx->heap.back()->offsets[ts_pnum] = bco+1;
			} else if(msg == ts_private) {
				data = &(ctx->heap.back()->num.vars[bco]);
				ctx->heap.back()->offsets[ts_num] = bco+1;
			}
		} else if(data) {
			num = ctx->findnum(ctx->code[i]);
			if(num) *data += *num;
				
			conv = nullptr;
		}
	}
	return __LINE__;
		
	/*if(action == a_decimal) return -__LINE__;
	std::wstring	vnam;
	int64_t		data = 0;
	int64_t			*num;
	toyFUNC			*conv = nullptr;
	for(int i = 0; i < csz; i++) {
		uint32_t	msg = code[i] & tsbcmsg;
		uint32_t	bco = code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(code[i]);
		} if(msg == tsbtype::ts_atxt && i == 0) {
			std::wstring	*str;
			str = ctx->findstr(code[i]);
			if(str) vnam = *str;
		} else {
			num = ctx->findnum(code[i]);
			if(num && i > 0) {
				data += *num;
			}
			conv = nullptr;
		}
	}
	if(vnam.size()>2) {
		std::wstring	cvnam;
		tsbtype				prudence = tsbtype::ts_pnum;
		if(vnam[0] == L'[') prudence = tsbtype::ts_num;
		cvnam.assign(vnam.c_str()+1, vnam.size()-2);
		ctx->allocnum(data, cvnam, prudence);
		return __LINE__;
	}*/
	return -__LINE__;
}
bool INTEGERtoy::okconvert(tsbcode cd) {
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	msg >>= 32UL-4UL;
	switch(msg) {
	case tsbtype::ts_adec:
	case tsbtype::ts_anum:
	//case tsbtype::ts_atxt:
	case tsbtype::ts_dec:
	case tsbtype::ts_num:
	//case tsbtype::ts_txt:
	case tsbtype::ts_pdec:
	case tsbtype::ts_pnum:
	//case tsbtype::ts_ptxt:
		return true;
	}

	return false;
}


TEXTtoy::TEXTtoy() {
	name = L"text";
	can_var = true;
	is_code = true;
	target_me = true;
	just_auto = true;
}

tsbcode TEXTtoy::do_just(toyCONTEXT *ctx, int it, size_t csz) {
	std::wstring	vnam;
	toyFUNC			*conv = nullptr;
	for(int i = it; i < it+csz; i++) {
		uint32_t	msg = ctx->code[i] & tsbcmsg;
		uint32_t	bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} if(msg == tsbtype::ts_atxt) {
			std::wstring	*str;
			str = ctx->findstr(ctx->code[i], it);
			if(str) {
				vnam = *str;
				if(vnam.size()>2) {
					std::wstring	cvnam;
					tsbtype				prudence = tsbtype::ts_ptxt;
					if(vnam[0] == L'[') prudence = tsbtype::ts_txt;
					cvnam.assign(vnam.c_str()+1, vnam.size()-2);
					tsbcode nc = ctx->alloctext(L"", cvnam, prudence);
					if(nc != invalidtsb) {
						uint32_t	s = 0;
						msg = nc & tsbcmsg;
						bco = nc &~tsbcmsg;
						msg >>= 32UL-4UL;
							
						if(prudence == ts_ptxt) {
							s = ctx->heap.back()->ptxt.vars.size();
							ctx->code[i] = mktsbc(bco, ts_protect);
						} else {
							s = ctx->heap.back()->txt.vars.size();
							ctx->code[i] = mktsbc(bco, ts_private);
						}
							
						ctx->heap.back()->offsets[prudence] = s;
						return invalidtsb;
					}
				}
			}
		}
	}
	return invalidtsb;
}

int TEXTtoy::func(toyCONTEXT *ctx, int it, size_t csz) {
	std::wstring	vnam;
	std::wstring	*data = nullptr;
	std::wstring			*num;
	toyFUNC			*conv = nullptr;
	for(int i = it; i < it+csz; i++) {
		uint32_t	msg = ctx->code[i] & tsbcmsg;
		uint32_t	bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} else if(data == nullptr) {
			if(msg == ts_protect) {
				data = &(ctx->heap.back()->ptxt.vars[bco]);
				ctx->heap.back()->offsets[ts_ptxt] = bco+1;
			} else if(msg == ts_private) {
				data = &(ctx->heap.back()->txt.vars[bco]);
				ctx->heap.back()->offsets[ts_txt] = bco+1;
			}
		}
		if(data) {
			num = ctx->findstr(ctx->code[i], it);
			if(num) {
				*data += *num;
			}
			conv = nullptr;
		}
	}
	return __LINE__;
}
bool TEXTtoy::okconvert(tsbcode cd) {
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	msg >>= 32UL-4UL;
	switch(msg) {
	case tsbtype::ts_cash:
	case tsbtype::ts_adec:
	case tsbtype::ts_anum:
	case tsbtype::ts_atxt:
	case tsbtype::ts_dec:
	case tsbtype::ts_num:
	case tsbtype::ts_txt:
	case tsbtype::ts_pdec:
	case tsbtype::ts_pnum:
	case tsbtype::ts_ptxt:
		return true;
	}

	return false;
}
std::wstring TEXTtoy::convert(toyCONTEXT *ctx, tsbcode code, int it) {
	uint32_t	msg = code & tsbcmsg;
	uint32_t	bco = code &~tsbcmsg;
	msg >>= 32UL-4UL;
		
	switch(msg) {
	const double	*dec;
	const int64_t	*num;
	std::wstring	*str;
	case tsbtype::ts_cash:
		num = ctx->getdollar();
		return std::to_wstring((num?*num:0));
	case tsbtype::ts_adec:
	case tsbtype::ts_dec:
	case tsbtype::ts_pdec:
		dec = ctx->finddec(code);
		return std::to_wstring((dec?*dec:0.0));
	case tsbtype::ts_anum:
	case tsbtype::ts_num:
	case tsbtype::ts_pnum:
		num = ctx->findnum(code);
		return std::to_wstring((num?*num:0));
	case tsbtype::ts_atxt:
	case tsbtype::ts_txt:
	case tsbtype::ts_ptxt:
		str = ctx->findstr(code, it);
		if(str) return *str;
	}
	return L"";
}

ASSIGNtoy::ASSIGNtoy(const wchar_t *pna) {
	name = pna;
	//can_var = true;
	if(name == L"==") {
		action = a_equals;
		as_accu = true;
	} else if(name == L"<") {
		action = a_lesser;
		as_accu = true;
	} else if(name == L">") {
		action = a_more;
		as_accu = true;
	} else if(name == L"**") {
		action = a_expose;
	} else if(name == L"^^") {
		action = a_square;
	} else if(name == L"%") {
		action = a_modulo;
		as_init = true;
	} else if(name == L"-") {
		as_init = true;
		action = a_minus;
	} else if(name == L"+") {
		as_init = true;
		action = a_plus;
	} else if(name == L"*") {
		as_init = true;
		action = a_mul;
	} else if(name == L"/") {
		as_init = true;
		action = a_div;
	} else if(name == L"TICKCOUNT") {
		action = a_tickcount;
			
	}
}
int ASSIGNtoy::do_d64(int64_t *d64, int64_t elem) {
	int	ret = 0;
	switch(action) {
	case a_equals: ret = (elem == *d64); break;
	case a_lesser: ret = (*d64 < elem); break;
	case a_more: ret = (*d64 > elem); break;
	case a_expose: *d64 += (elem*elem); break;
	case a_square: *d64 += sqrt((double)elem); break;
	case a_modulo: if(elem > 0) *d64 = *d64 % elem; break;
	case a_minus: *d64 -= elem; break;
	case a_plus: *d64 += elem; break;
	case a_mul: *d64 *= elem; break;
	case a_div: if(elem != 0) *d64 /= elem; break;
#ifdef _WINDOWS_
	case a_tickcount: *d64 = GetTickCount64(); break;
#elif	__unix__
	case a_tickcount: {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		*d64 = (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);
	} break;
#endif

	default: *d64 = elem; break;
	}
	return ret;
}
int ASSIGNtoy::do_f64(double *d64, double elem) {
	int	ret = 0;
	switch(action) {
	case a_equals: ret = (elem == *d64); break;
	case a_lesser: ret = (*d64 < elem); break;
	case a_more: ret = (*d64 > elem); break;
	case a_expose: *d64 += (elem*elem); break;
	case a_square: *d64 += sqrt(elem); break;
	case a_minus: *d64 -= elem; break;
	case a_plus: *d64 += elem; break;
	case a_mul: *d64 *= elem; break;
	case a_div: if(elem != 0) *d64 /= elem; break;
	default: *d64 = elem; break;
	}
	return ret;
}
int ASSIGNtoy::do_dstr(std::wstring *d64, const std::wstring &elem) {
	int	ret = 0;
	switch(action) {
	case a_equals: ret = (elem == *d64); break;
	case a_assign: *d64 = elem; break;
	default: *d64 += elem; break;
	}
	return ret;
}
int ASSIGNtoy::func(toyCONTEXT *ctx, int it, size_t csz) {
	it++;
	csz--;
	uint32_t	msg = ctx->code[it] & tsbcmsg;
	uint32_t	bco = ctx->code[it] &~tsbcmsg;
	std::wstring	*dstr = nullptr;
	int64_t		*d64 = nullptr;
	double		*f64 = nullptr;

	msg >>= 32UL-4UL;
	if(ctx->code[it] == mktsbc(0, ts_cash)) {
		d64 = ctx->getdollar();
	} else if(msg == ts_num || msg == ts_pnum) {
		d64 = ctx->findnum(ctx->code[it], (tsbtype)msg);
	} else if(msg == ts_dec || msg == ts_pdec) {
		f64 = ctx->finddec(ctx->code[it], (tsbtype)msg);
	} else if(msg == ts_txt || msg == ts_ptxt) {
		dstr = ctx->findstr(ctx->code[it], (tsbtype)msg);
	}
		
	int64_t			*t64;
	double			*l64;
	std::wstring	saccu = L"";
	double			daccu = 0.0;
	int64_t			iaccu = 0;
	toyFUNC			*conv = nullptr;
	int				count = 0;
	if(as_accu || as_init) {
		if(d64) iaccu = *d64;
		if(f64) daccu = *f64;
		if(dstr) saccu = *dstr;
	}
	if(action == a_tickcount){
		#ifdef _WINDOWS_
		iaccu = GetTickCount64();
#elif	__unix__
	
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		iaccu = (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);
	
#endif
		//iaccu = GetTickCount64();
	}
	int64_t		off1, off2, *poff1 = nullptr, *poff2 = nullptr;
	off1 = 0;
	off2 = -1;
	std::wstring	*pcut = nullptr;
	for(int i = it+1; i < it+csz; i++) {
		uint32_t	msg = ctx->code[i] & tsbcmsg;
		uint32_t	bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} else {
			bool concan = false;
			if(d64) {
				if(concan == false) {
					t64 = ctx->findnum(ctx->code[i]);
					if(t64) count += do_d64(&iaccu, *t64);
					else {
						l64 = ctx->finddec(ctx->code[i]);
						if(l64) count += do_d64(&iaccu, *l64);
					}
				}
			} else if(f64) {
				if(concan == false) {
					l64 = ctx->finddec(ctx->code[i]);
					if(l64) count += do_f64(&daccu, *l64);
					else {
						t64 = ctx->findnum(ctx->code[i]);
						if(t64) count += do_f64(&daccu, *t64);
					}
				}
			} else if(dstr) {
				if(conv) {
					if(conv->okconvert(ctx->code[i])) {
						concan = true;
						count += do_dstr(&saccu, conv->convert(ctx, ctx->code[i], it-1));
					}
				} else {
					if(poff1 == nullptr) {
						poff1 = ctx->findnum(ctx->code[i]);
					} else if(poff2 == nullptr) {
						poff2 = ctx->findnum(ctx->code[i]);
					}
				}
				if(concan == false && poff1 == nullptr) {
					std::wstring	*ts;
					ts = ctx->findstr(ctx->code[i], it);
					if(ts) count += do_dstr(&saccu, *ts);
				} else if(concan == false && poff1 != nullptr) {
					if(pcut == nullptr)
						pcut = ctx->findstr(ctx->code[i], it);
				}
			}
			conv = nullptr;
		}
	}
	if(pcut && dstr) {
		if(poff1 == nullptr) {
			poff1 = &off1;
		}
		if(poff2 == nullptr) {
			poff2 = &off2;
		}
		if(*poff2 == -1) {
			*poff2 = pcut->size();
		}
		uint64_t ll;
		ll = (*poff2)-(*poff1);
		if((*poff1)+ll > pcut->size()) {
			ll = 0;
			*poff1 = 0;
		}
		dstr->assign(*pcut, *poff1, ll);
		return -__LINE__;
	}
	if(d64) {
		if(as_accu) iaccu = count;
		*d64 = iaccu;
	} else if(f64) {
		if(as_accu) daccu = count;
		*f64 = daccu;
	} else if(dstr) {
		if(as_accu) {
			d64 = ctx->getdollar();
			if(d64) *d64 = count;
		} else {
			*dstr = saccu;
		}
	}
	return -__LINE__;
}



PRINTtoy::PRINTtoy() {
	name = L"print";
}
int PRINTtoy::func(toyCONTEXT *ctx, int it, size_t csz) {
	std::wstring	rere;
	std::wstring	*str;
	toyFUNC			*conv = nullptr;
	for(int i = it+1; i < it+csz; i++) {
		uint32_t	msg = ctx->code[i] & tsbcmsg;
		uint32_t	bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} else {
			bool	cancon = false;
			if(conv) {
				if(conv->okconvert(ctx->code[i])) {
					cancon = true;
					rere += conv->convert(ctx, ctx->code[i], it);
				}
			}
			if(cancon == false) {
				str = ctx->findstr(ctx->code[i], it);
				if(str) {
					rere += *str;
				}
			}
			conv = nullptr;
		}
	}
	std::wcout << rere << std::flush;
	return __LINE__;
}

FINDtoy::FINDtoy(const wchar_t *pna) {
	name = pna;
	if(name == L"count") {
		action = f_count;
	}
}

int FINDtoy::func(toyCONTEXT *ctx, int it, size_t csz) {
	it++;
	csz--;
	uint32_t	msg = ctx->code[it] & tsbcmsg;
	uint32_t	bco = ctx->code[it] &~tsbcmsg;
	std::wstring	*dstr = nullptr;
	int64_t		*d64 = nullptr;
	double		*f64 = nullptr;

	msg >>= 32UL-4UL;
	if(msg == ts_cash) {
		d64 = ctx->getdollar();
	} else if(msg == ts_num || msg == ts_pnum) {
		d64 = ctx->findnum(ctx->code[it], (tsbtype)msg);
	} else if(msg == ts_dec || msg == ts_pdec) {
		f64 = ctx->finddec(ctx->code[it], (tsbtype)msg);
	} else if(msg == ts_txt || msg == ts_ptxt) {
		dstr = ctx->findstr(ctx->code[it], (tsbtype)msg);
	}
		
//	int64_t			*t64;
	//double			*l64;
	
	toyFUNC			*conv = nullptr;
	int				count = 0;
	int64_t			*ieie = nullptr;
	double			*dede = nullptr;
	std::wstring	*sese = nullptr;
	std::wstring	csese;
	std::wstring	*sese2 = nullptr;
	std::wstring	csese2;
	bool			oksese = false;
	bool			oksese2 = false;
	for(int i = it+1; i < it+csz; i++) {
		uint32_t	msg = ctx->code[i] & tsbcmsg;
		uint32_t	bco = ctx->code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(ieie && oksese && oksese2) {
			break;
		}
		if(msg == tsbtype::ts_func) {
			conv = ctx->findtoy(ctx->code[i]);
		} else {
			switch(msg) {
			case ts_anum: case ts_num: case ts_pnum:
				if(conv) {
					if(conv->okconvert(ctx->code[i])) {
						if(oksese == false) {
							csese = conv->convert(ctx, ctx->code[i], it-1);
							oksese = true;
						} else if(oksese2 == false) {
							csese2 = conv->convert(ctx, ctx->code[i], it-1);
							oksese2 = true;
						}
					}
				} else if(ieie == nullptr) {
					ieie = ctx->findnum(ctx->code[i]);
				}
				break;
			case ts_adec: case ts_dec: case ts_pdec:
				if(conv) {
					if(conv->okconvert(ctx->code[i])) {
						if(oksese == false) {
							csese = conv->convert(ctx, ctx->code[i], it-1);
							oksese = true;
						} else if(oksese2 == false) {
							csese2 = conv->convert(ctx, ctx->code[i], it-1);
							oksese2 = true;
						}
					}
				} else if(dede == nullptr) {
					dede = ctx->finddec(ctx->code[i]);
				}
				break;
			case ts_atxt: case ts_txt: case ts_ptxt:
				if(oksese == false) {
					sese = ctx->findstr(ctx->code[i], it-1);
					oksese = true;
				} else if(oksese2 == false) {
					sese2 = ctx->findstr(ctx->code[i], it-1);
					oksese2 = true;
				}
				break;
			}
			conv = nullptr;
		}
	}
	if(d64) {
		*d64 = -1;
	}
	if(oksese && oksese2) {
		if(sese == nullptr) {
			sese = &csese;
		}
		if(sese2 == nullptr) {
			sese2 = &csese2;
		}
		int64_t	ueue = 0;
		if(ieie == 0) {
			ieie = &ueue;
		}
		if(ieie && sese2->size()==1) {
			if(d64) {
				size_t	rt;
				rt = sese->find(sese2[0], *ieie);
				if(rt != std::wstring::npos) {
					*d64 = rt;
				}
			}
		}
	}
	return -__LINE__;
}

COUNTtoy::COUNTtoy():funcGAME(L"count") {

}

void COUNTtoy::withNUM(cgame *game) {
	while(game->domore()) {
		std::wstring	*str;
		str = game->getstring();
		if(str) {
			*game->cip = str->size();
			return;
		}
	}
	*game->cip = 0;
}

///////////////////////////////

COMPAREtoy::COMPAREtoy(std::wstring str):funcGAME(str.c_str()) {
	
}

void COMPAREtoy::compare(cgame *game, std::wstring *s1, std::wstring *s2) {
	if(game->cip) {
		switch(action) {
			case c_equals:	*game->cip = (*s1 == *s2); break;
		}
	}
}

void COMPAREtoy::compare(cgame *game, int64_t s1, int64_t s2) {
	if(game->cip) {
		switch(action) {
			case c_equals:	*game->cip = (s1 == s2); break;
		}
	}
}

void COMPAREtoy::compare(cgame *game, double s1, double s2) {
	if(game->cip) {
		switch(action) {
			case c_equals:	*game->cip = (s1 == s2); break;
		}
	}
}

void COMPAREtoy::withSTR(cgame *game) {
	game->cip = game->ctx->getdollar();
	if(game->cip) *game->cip = 0;
	while(game->domore()) {
		if(game->what() == ts_txt) {
			std::wstring	*str;
			str = game->getstring();
			if(game->cis && str) {
				compare(game, game->cis, str);
			}
		} else {
			game->ignoreone();
		}
	}
}

void COMPAREtoy::withNUM(cgame *game) {
	std::wstring	*str = nullptr;
	int64_t			*num = nullptr;
	double			*dec = nullptr;
	tsbtype			wish = ts_stop;
	if(game->isdollar == false) {
		if(game->cip && game->cip == 0) {
			return;
		}
	}
	while(game->domore()) {
		if(wish == ts_stop) {
			wish = game->what();
			switch(wish) {
			case ts_txt: str = game->getstring(); break;
			case ts_num: num = game->getinteger(); break;
			case ts_dec: dec = game->getdecimal(); break;
			}
		} else {
			std::wstring	*sstr = nullptr;
			int64_t			*snum = nullptr;
			double			*sdec = nullptr;
			switch(wish) {
			case ts_txt: sstr = game->getstring(); break;
			case ts_num: snum = game->getinteger(); break;
			case ts_dec: sdec = game->getdecimal(); break;
			}
			switch(wish) {
			case ts_txt: if(str && sstr) compare(game, str, sstr); break;
			case ts_num: if(num && snum) compare(game, *num, *snum); break;
			case ts_dec: if(dec && sdec) compare(game, *dec, *sdec); break;
			}
			if(sstr) str = sstr;
			if(snum) num = snum;
			if(sdec) dec = sdec;
		}
	}
}