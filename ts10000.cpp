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

toyIMMEDIAT::toyIMMEDIAT() {
	parent = nullptr;
	distance = 0;
	for(int i = 0; i < 9; i++) {
		offsets[i] = 0;
	}
	func_offset = subs_offset = 0;
	code_position = -1;
}

void toyIMMEDIAT::branchthrough(toyCONTEXT *ctx) {
	
	uint32_t	coffsets[9];
	uint32_t	cfunc_offset;
	uint32_t	csubs_offset;

	toyHEAP		*hp = parent;
	for(int i = 0; i < 9; i++) {
		coffsets[i] = hp->offsets[i];
		hp->offsets[i] = offsets[i];
	}
	cfunc_offset = hp->func_offset;
	hp->func_offset = func_offset;
	csubs_offset = hp->subs_offset;
	hp->subs_offset = subs_offset;

	std::vector	<toyHEAP*>	backed;
	int	disto = -1;
	for(int i = 0; i < ctx->heap.size(); i++) {
		if(ctx->heap[i] == parent) {
			disto = i+1;
			break;
		}
	}
	if(disto != -1) {
		for(int i = ctx->heap.size(); i > disto; i--) {
			backed.push_back(ctx->heap.back());
			ctx->heap.pop_back();
		}
	}
	uint32_t	msg = ctx->code[code_position] & tsbcmsg;
	uint32_t	bco = ctx->code[code_position] &~tsbcmsg;
	msg >>= 32UL-4UL;
	if(msg == ts_stop) {
		if(prev) prev->branchthrough(ctx);
		tsbcode savedfunc = ctx->code[code_position+1];
		ctx->code[code_position+1] = backfunc;
		if(mac) mac->execline(code_position);
		ctx->code[code_position+1] = savedfunc;
	}

	while(backed.size()) {
		ctx->heap.push_back(backed.back());
		backed.pop_back();
	}
	
	hp->func_offset = cfunc_offset;
	hp->subs_offset = csubs_offset;
	for(int i = 0; i < 9; i++) {
		hp->offsets[i] = coffsets[i];
	}

}

toyCODE::toyCODE() {
	
}

int toyCODE::code(toyMACHINE *mac, int it, size_t csz) {

	return -__LINE__;
}


tsbcode toyFUNC::do_just(toyCONTEXT *ctx, int it, size_t csz) {
	return invalidtsb;
}
int toyFUNC::func(toyCONTEXT *ctx, int it, size_t csz) {
	return -__LINE__;
}
bool toyFUNC::okconvert(tsbcode cd) {
	return false;
}
std::wstring toyFUNC::convert(toyCONTEXT *ctx, tsbcode code, int it) {
	return L"";
}


template <typename T>
int toyVARS<T>::getpos(std::wstring src, int offset) {
	int	ret;
	for(int i = offset; i > 0; i--) {
		if(names[i-1].compare(src)==0) {
			return i-1;
		}
	}
	return -1;
}



toyHEAP::toyHEAP() {
	offsets[0] = 0;
	offsets[1] = 0;
	offsets[2] = 0;
	offsets[3] = 0;
	offsets[4] = 0;
	offsets[5] = 0;
	offsets[6] = 0;
	offsets[7] = 0;
	offsets[8] = 0;
	dollar = 0;
	func_offset = 0;
	subs_offset = 0;
}
tsbcode toyHEAP::getcorrect(std::wstring src) {
	int	ret;

	ret = pnum.getpos(src, offsets[tsbtype::ts_pnum]);
	if(ret >= 0) return mktsbc(ret, tsbtype::ts_pnum);
	ret = pdec.getpos(src, offsets[tsbtype::ts_pdec]);
	if(ret >= 0) return mktsbc(ret, tsbtype::ts_pdec);
	ret = ptxt.getpos(src, offsets[tsbtype::ts_ptxt]);
	if(ret >= 0) return mktsbc(ret, tsbtype::ts_ptxt);

	return invalidtsb;
}
tsbcode toyHEAP::getasgnpos(std::wstring src, bool known_p) {
	int	ret;
	if(known_p == false) {
		ret = num.getpos(src, offsets[tsbtype::ts_num]);
		if(ret >= 0) return mktsbc(ret, tsbtype::ts_num);
		ret = dec.getpos(src, offsets[tsbtype::ts_dec]);
		if(ret >= 0) return mktsbc(ret, tsbtype::ts_dec);
		ret = txt.getpos(src, offsets[tsbtype::ts_txt]);
		if(ret >= 0) return mktsbc(ret, tsbtype::ts_txt);
	}

	return getcorrect(src);
}


toyCONTEXT::toyCONTEXT() {
	heap.push_back(new toyHEAP());
}

void toyCONTEXT::codepush(tsbcode cd) {
	code.push_back(cd);
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	msg >>= 32UL-4UL;
	//std::wcout << msg << L":" << bco << L"\n";
	//std::cout << cd << std::endl;
}

void toyCONTEXT::prepare() {
	heap.back()->offsets[0] =
	heap.back()->offsets[1] =
	heap.back()->offsets[2] =
	heap.back()->offsets[3] =
	heap.back()->offsets[4] =
	heap.back()->offsets[5] = 0;
	heap.back()->dollar = 0;
	code_position = 0;
}

int64_t *toyCONTEXT::getdollar() {
	if(heap.size() > 0) {
		return &(heap.back()->dollar);
	}
	return nullptr;
}
tsbcode toyCONTEXT::startresolve(std::wstring vnam) {
	if(vnam == L"$") return mktsbc(0, ts_cash);
	if(vnam == L"#") return mktsbc(0, ts_cash);
	if(heap.size()==0) return invalidtsb;
	std::wstring cnam = vnam;
	bool known_p = false;
	if(vnam.size()>2) {
		if(vnam[0] == L'<') known_p = true;
		if(vnam[0] == L'[' || known_p == true) {
			cnam.assign(vnam.c_str()+1, vnam.size()-2);
		}
	}
	return heap[heap.size()-1]->getasgnpos(cnam, known_p);
}
tsbcode toyCONTEXT::resolvevar(std::wstring vnam) {
	tsbcode		ret;
	std::wstring cnam;
	ret = startresolve(vnam);
	if(ret != invalidtsb) return ret;
	uint32_t	offsets[9] = {0,0,0,0,0,0,0,0,0};
	if(vnam.size()>2 && (vnam[0] == L'<' || vnam[0] == L'['))
		cnam.assign(vnam.c_str()+1, vnam.size()-2);
	else cnam = vnam;
	for(size_t i = heap.size(); i > 0; i--) {
		//if(i == heap.size())
		//	ret = heap[i-1]->getasgnpos(cnam, false);
		//else
			ret = heap[i-1]->getasgnpos(cnam);
		if(ret == invalidtsb) {
			offsets[tsbtype::ts_num] += heap[i-1]->offsets[tsbtype::ts_num];
			offsets[tsbtype::ts_dec] += heap[i-1]->offsets[tsbtype::ts_dec];
			offsets[tsbtype::ts_txt] += heap[i-1]->offsets[tsbtype::ts_txt];
			offsets[tsbtype::ts_pnum] += heap[i-1]->offsets[tsbtype::ts_pnum];
			offsets[tsbtype::ts_pdec] += heap[i-1]->offsets[tsbtype::ts_pdec];
			offsets[tsbtype::ts_ptxt] += heap[i-1]->offsets[tsbtype::ts_ptxt];
		} else {
			uint32_t	msg = ret & tsbcmsg;
			uint32_t	bco = ret &~tsbcmsg;
			msg >>= 32UL-4UL;
			bco += offsets[msg];
			ret = mktsbc(bco, msg);
			return ret;
		}
	}
	return invalidtsb;
}

double *toyCONTEXT::finddec(tsbcode cd, tsbtype tp) {
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	msg >>= 32UL-4UL;
	if(msg == tsbtype::ts_adec) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_adec]) {
				return &(heap[i-1]->anon_dec[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_adec];
		}
	} else if(msg == tsbtype::ts_dec) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_dec]) {
				return &(heap[i-1]->dec.vars[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_dec];
		}
	} else if(msg == tsbtype::ts_pdec) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_pdec]) {
				return &(heap[i-1]->pdec.vars[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_pdec];
		}
	}
	return nullptr;
}

int64_t *toyCONTEXT::findnum(tsbcode cd, tsbtype tp) {
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	msg >>= 32UL-4UL;
	if(msg == ts_cash) {
		return getdollar();
	}
	if(msg == tsbtype::ts_anum) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_anum]) {
				return &(heap[i-1]->anon_num[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_anum];
		}
	} else if(msg == tsbtype::ts_num) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_num]) {
				return &(heap[i-1]->num.vars[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_num];
		}
	} else if(msg == tsbtype::ts_pnum) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_pnum]) {
				return &(heap[i-1]->pnum.vars[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_pnum];
		}
	}
	return nullptr;
}



bool toyCONTEXT::test_ok_exec(tsbcode okc, toyHEAP *opth, int it) {
	if(okc == invalidtsb) return false;
	
	toyPAIR	*fnd = nullptr, *pfnd = nullptr;
	int		uu = 0;
	fnd = imme;
	while(fnd) {
		if(fnd->okc == okc) { // && fnd->othp == opth) {
			break;
		}
		pfnd = fnd;
		fnd = fnd->next;
	}
	if(fnd && fnd->ti) {
		toyIMMEDIAT	*pti = nullptr, *ti;
		ti = fnd->ti;
		while(ti) {
			if(ti->code_position < it) {
				break;
			}
			pti = ti;
			ti = ti->prev;
		}
		if(ti) {
			if(pti) pti->prev = nullptr;
			fnd->ti = pti;
			pti = ti;
			if(it > ti->code_position) {
				if(fnd->ti == ti) {
					if(pfnd) {
						pfnd->next = fnd->next;
					} else {
						imme = fnd->next;
					}
				}
				//fnd->next = nullptr;
				ti->branchthrough(this);
				toyIMMEDIAT	*ti2;
				while(ti) {
					ti2 = ti->prev;
					delete ti;
					ti = ti2;
				}
				if(fnd->ti == pti)
					delete fnd;
				return true;
			}
		}
	} else {
		toyPAIR		*pimme = imme;
		if(isso.size()>0) {
			imme = isso.back();
			isso.pop_back();
			if(test_ok_exec(okc, opth, it)) {
				isso.push_back(imme);
				return true;
			} else {
				isso.push_back(pimme);
			}
		}
	}
	/*for(auto i = imme.begin(); i != imme.end();uu++, i++) {
		if( (*i)->okc == okc && (*i)->othp == opth) {
			fnd = *i;
			break;
		}
	}
	if(fnd && fnd->ti) {
		if(it > fnd->ti->code_position) {
			imme.erase(imme.begin()+uu);
			fnd->ti->branchthrough(this);
			toyIMMEDIAT	*ti2;
			while(fnd->ti) {
				ti2 = fnd->ti->prev;
				delete fnd->ti;
				fnd->ti = ti2;
			}
			delete fnd;
		}
	}*/
	return false;
}

std::wstring *toyCONTEXT::findstr(tsbcode cd, int it, tsbtype tp, bool oktest, toyHEAP **opth, tsbcode *okc) {
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	tsbcode	kc; toyHEAP *pth;
	msg >>= 32UL-4UL;
	if(msg == tsbtype::ts_atxt) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_atxt]) {
				if(opth) *opth = heap[i-1];
				if(okc) *okc = mktsbc(bco, msg);
				return &(heap[i-1]->anon_str[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_atxt];
		}
	} else if(msg == tsbtype::ts_txt) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_txt]) {
				pth = heap[i-1]; if(opth) *opth = pth;
				kc = mktsbc(bco, msg); if(okc) *okc = kc;
				if(oktest) test_ok_exec(kc, pth, it);
				return &(heap[i-1]->txt.vars[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_txt];
		}
	} else if(msg == tsbtype::ts_ptxt) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->offsets[tsbtype::ts_ptxt]) {
				pth = heap[i-1]; if(opth) *opth = pth;
				kc = mktsbc(bco, msg); if(okc) *okc = kc;
				if(oktest) test_ok_exec(kc, pth, it);
				return &(heap[i-1]->ptxt.vars[bco]);
			}
			bco -= heap[i-1]->offsets[tsbtype::ts_ptxt];
		}
	}
	return nullptr;
}

toyFUNC *toyCONTEXT::findtoy(tsbcode cd) {
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	msg >>= 32UL-4UL;

	if(msg == tsbtype::ts_func) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->func_offset) {
				return (heap[i-1]->func[bco]);
			}
			if(i > 1)
				bco -= heap[i-2]->func_offset;
			
		}
	}
	return nullptr;
	/*int	off = cd & ~tsbcmsg;
	if(off>=0 && off<func.size()) {
		return func[off];
	}
	return nullptr;*/
}
toyCODE *toyCONTEXT::findsub(tsbcode cd) {
	toyCODE		*ret = nullptr;
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	msg >>= 32UL-4UL;

	if(msg == tsbtype::ts_code) {
		for(size_t i = heap.size(); i > 0; i--) {
			if(bco < heap[i-1]->subs.size()) {
				ret = (heap[i-1]->subs[bco]);
				if(ret) {
					if(i < heap.size()) {
						if(ret->prudence == ts_private) {
							//ret = nullptr;
						}
					}
				}
				return ret;
			}
			if(i > 1)
				bco -= heap[i-2]->subs_offset;
		}
	}
	return nullptr;
	/*int	off = cd & ~tsbcmsg;
	if(off>=0 && off<subs.size()) {
		return subs[off];
	}
	return nullptr;*/
}
tsbcode toyCONTEXT::getsubpos(std::wstring src) {
	tsbcode	ret;
	uint32_t	offset = 0, offset2 = 0;
	for(int i = heap.size(); i > 0; i--) {
		for(int u = heap[i-1]->subs_offset; u > 0; u--) {
			if(heap[i-1]->subs[u-1]->name == src) {
				if(i < heap.size()) {
					if(heap[i-1]->subs[u-1]->prudence == ts_private) {
						continue;
					}
				}
				return mktsbc((u-1)+offset, tsbtype::ts_code);
			}
		}
		if(i > 1) offset2 = heap[i-2]->subs_offset;
		offset += offset2;
		offset2 = 0;
	}
	return invalidtsb;
	/*for(int i = subs_offset; i > 0; i--) {
		if(subs[i-1]->name == src) {
			return mktsbc(i-1, tsbtype::ts_code);
		}
	}
	return invalidtsb;*/
}
tsbcode toyCONTEXT::getfuncpos(std::wstring src) {
	tsbcode	ret;
	uint32_t	offset = 0, offset2 = 0;
	for(int i = heap.size(); i > 0; i--) {
		for(int u = heap[i-1]->func.size(); u > 0; u--) {
			if(heap[i-1]->func[u-1]->name == src) {
				return mktsbc((u-1)+offset, tsbtype::ts_func);
			}
		}
		if(i > 1) offset2 = heap[i-2]->func_offset;
		offset += offset2;
		offset2 = 0;
		//offset += heap[i-1]->func_offset;
	}
	return invalidtsb;
	/*
	for(int i = func_offset; i > 0; i--) {
		if(func[i-1]->name == src) {
			return mktsbc(i-1, tsbtype::ts_func);
		}
	}
	return invalidtsb;*/
}
tsbcode toyCONTEXT::getasgnpos(std::wstring src, bool known_p) {
	tsbcode	ret;
	uint32_t	offsets[6] = {0,0,0,0,0,0};
	for(int i = heap.size(); i > 0; i--) {
		ret = heap[i-1]->getasgnpos(src, known_p);
		if(ret != invalidtsb) {
			uint32_t	msg = ret & tsbcmsg;
			uint32_t	bco = ret &~tsbcmsg;
			msg >>= 32UL-4UL;
			if(msg < 6) {
				bco += offsets[msg];
			}
			return mktsbc(bco, msg);
		}
		offsets[0] += heap[i-1]->offsets[0];
		offsets[1] += heap[i-1]->offsets[1];
		offsets[2] += heap[i-1]->offsets[2];
		offsets[3] += heap[i-1]->offsets[3];
		offsets[4] += heap[i-1]->offsets[4];
		offsets[5] += heap[i-1]->offsets[5];
	}
	return invalidtsb;
}

tsbcode toyCONTEXT::allocnum(int64_t src, std::wstring vnam, tsbtype msg) {
	tsbcode	ret = invalidtsb;

	if(heap.size()>0) {
		toyHEAP	*hp = heap[heap.size()-1];
		if(msg == tsbtype::ts_pnum) {
			ret = mktsbc(hp->pnum.vars.size(), msg);
			hp->pnum.names.push_back(vnam);
			hp->pnum.vars.push_back(src);
			hp->offsets[msg] = hp->pnum.vars.size();
		} else if(msg == tsbtype::ts_num) {
			ret = mktsbc(hp->num.vars.size(), msg);
			hp->num.names.push_back(vnam);
			hp->num.vars.push_back(src);
			hp->offsets[msg] = hp->num.vars.size();
		}
	}
	return ret;
}

tsbcode toyCONTEXT::alloccode(toyCODE *mc) {
	tsbcode ret = invalidtsb;

	if(heap.size()>0) {
		toyHEAP	*hp = heap[heap.size()-1];
		uint32_t	acsz = 0;
		for(int i = heap.size(); i > 0; i--) {
			acsz += heap[i-1]->subs.size();
		}
		
		ret = mktsbc(hp->subs.size(), ts_code);
		hp->subs.push_back(mc);
		hp->subs_offset = hp->subs.size();
		heap.push_back(&mc->sheap);
		mc->immcon.parent = hp;
		mc->immcon.distance = heap.size()-1;
		//hp->subs_offset++;
		for(int i = 0; i < 9; i++) {
			mc->immcon.offsets[i] = hp->offsets[i];
		}
		mc->immcon.func_offset = hp->func_offset;
		mc->immcon.subs_offset = hp->subs_offset;
		isso.push_back(imme);
		mc->immcon.imme = 0;
		imme = nullptr;
	}
	if(ret == invalidtsb) delete mc;
	return ret;
}

tsbcode toyCONTEXT::allocdec(double src, std::wstring vnam, tsbtype msg) {
	tsbcode	ret = invalidtsb;

	if(heap.size()>0) {
		toyHEAP	*hp = heap[heap.size()-1];
		if(msg == tsbtype::ts_pdec) {
			ret = mktsbc(hp->pdec.vars.size(), msg);
			hp->pdec.names.push_back(vnam);
			hp->pdec.vars.push_back(src);
			hp->offsets[msg] = hp->pdec.vars.size();
		} else if(msg == tsbtype::ts_dec) {
			ret = mktsbc(hp->dec.vars.size(), msg);
			hp->dec.names.push_back(vnam);
			hp->dec.vars.push_back(src);
			hp->offsets[msg] = hp->dec.vars.size();
		}
	}
	return ret;
}

tsbcode toyCONTEXT::alloctext(std::wstring src, std::wstring vnam, tsbtype msg) {
	tsbcode	ret = invalidtsb;

	if(heap.size()>0) {
		toyHEAP	*hp = heap[heap.size()-1];
		if(msg == tsbtype::ts_ptxt) {
			ret = mktsbc(hp->ptxt.vars.size(), msg);
			hp->ptxt.names.push_back(vnam);
			hp->ptxt.vars.push_back(src);
			hp->offsets[msg] = hp->ptxt.vars.size();
		} else if(msg == tsbtype::ts_txt) {
			ret = mktsbc(hp->txt.vars.size(), msg);
			hp->txt.names.push_back(vnam);
			hp->txt.vars.push_back(src);
			hp->offsets[msg] = hp->txt.vars.size();
		}
	}
	return ret;
}

tsbcode toyCONTEXT::allocanondec(double dec) {
	tsbcode	ret;

	if(heap.size()>0) {
		toyHEAP	*hp = heap[heap.size()-1];
		ret = mktsbc(hp->anon_dec.size(), tsbtype::ts_adec);
		hp->anon_dec.push_back(dec);
		hp->offsets[tsbtype::ts_adec] = hp->anon_dec.size();
		return ret;
	}
	return invalidtsb;
}
tsbcode toyCONTEXT::allocanonnum(int64_t num) {
	tsbcode	ret;

	if(heap.size()>0) {
		toyHEAP	*hp = heap[heap.size()-1];
		ret = mktsbc(hp->anon_num.size(), tsbtype::ts_anum);
		hp->anon_num.push_back(num);
		hp->offsets[tsbtype::ts_anum] = hp->anon_num.size();
		return ret;
	}
	return invalidtsb;
}
tsbcode toyCONTEXT::allocanonstr(std::wstring src) {
	tsbcode	ret;

	if(heap.size()>0) {
		toyHEAP	*hp = heap[heap.size()-1];
		ret = mktsbc(hp->anon_str.size(), tsbtype::ts_atxt);
		hp->anon_str.push_back(src);
		hp->offsets[tsbtype::ts_atxt] = hp->anon_str.size();
		return ret;
	}
	return invalidtsb;
}

////////////////////////////////////////////

cgame::cgame(toyCONTEXT *pctx, int pti, size_t pcsz) {
	ctx = pctx; it = pti; csz = pcsz;
	scsz = csz;
}

tsbtype cgame::identify() {
	it++;
	csz--;
	uint32_t	msg, bco;
	msg = ctx->code[it] & tsbcmsg;
	bco = ctx->code[it] &~tsbcmsg;
	msg >>= 32UL-4UL;

	it++;
	csz--;

	if(msg == ts_cash) {
		cip = ctx->getdollar();
		return ts_num;
	} else if(msg == ts_num || msg == ts_pnum) {
		cip = ctx->findnum(ctx->code[it-1], (tsbtype)msg);
		return ts_num;
	} else if(msg == ts_dec || msg == ts_pdec) {
		cid = ctx->finddec(ctx->code[it-1], (tsbtype)msg);
		return ts_dec;
	} else if(msg == ts_txt || msg == ts_ptxt) {
		cis = ctx->findstr(ctx->code[it-1], (tsbtype)msg);
		return ts_txt;
	}
	return ts_stop;
}

bool cgame::domore() {
	if(scsz == csz) return false;
	scsz = csz;
	if(csz > 0) return true;
	return false;
}

std::wstring *cgame::getstring() {
	std::wstring *ret;
	ret = ctx->findstr(ctx->code[it], it);
	if(ret) {
		it++; csz--;
	}
	return ret;
}

funcGAME::funcGAME(const wchar_t *pna) {
	name = pna;
}

int funcGAME::func(toyCONTEXT *ctx, int it, size_t csz) {
	cgame		game(ctx, it, csz);
	switch(game.identify()) {
	case ts_txt: this->withSTR(&game); break;
	case ts_dec: this->withDEC(&game); break;
	case ts_num: this->withNUM(&game); break;
	}
	return __LINE__;

}

void funcGAME::withSTR(cgame *game) {

}

void funcGAME::withDEC(cgame *game) {

}

void funcGAME::withNUM(cgame *game) {

}

//////

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
	std::wcout << rere;
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
		
	int64_t			*t64;
	double			*l64;
	
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
	if(msg == ts_cash) {
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
		iaccu = GetTickCount64();
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
		int32_t ll;
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
	


IFcode::IFcode() {
	name = L"if";
	cant_just = true;
}

LOOPcode::LOOPcode() {
	name = L"loop";
	cant_just = true;
}

class MACHINEtoy : public toyCODE {
public:
	MACHINEtoy() {
		name = L"@";
	}
	int code(toyMACHINE *mac, int it, size_t csz) {
		int	i;
		for(i = it; i < it+csz; i++) {
			uint32_t	msg = mac->context.code[i] & tsbcmsg;
			uint32_t	bco = mac->context.code[i] &~tsbcmsg;
			msg >>= 32UL-4UL;
			if(msg == ts_stop) {
				break;
			}
		}
		return i-it;
	}
};

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
	if(msg == ts_cash) {
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

toyMACHINE::toyMACHINE() {
	std::vector<toyFUNC*>	*func;
	func = &(context.heap.back()->func);
	(*func).push_back(new PRINTtoy());
	(*func).push_back(new INPUTtoy());
	(*func).push_back(new CONVERTtoy());
	(*func).push_back(new TEXTtoy());
	(*func).push_back(new COUNTtoy());
	(*func).push_back(new INTEGERtoy());
	(*func).push_back(new INTEGERtoy(L"decimal"));
	(*func).push_back(new FINDtoy());
	(*func).push_back(new ASSIGNtoy());
	(*func).push_back(new ASSIGNtoy(L"=="));
	(*func).push_back(new ASSIGNtoy(L"<"));
	(*func).push_back(new ASSIGNtoy(L">"));
	(*func).push_back(new ASSIGNtoy(L"**"));
	(*func).push_back(new ASSIGNtoy(L"^^"));
	(*func).push_back(new ASSIGNtoy(L"%"));
	(*func).push_back(new ASSIGNtoy(L"-"));
	(*func).push_back(new ASSIGNtoy(L"+"));
	(*func).push_back(new ASSIGNtoy(L"*"));
	(*func).push_back(new ASSIGNtoy(L"/"));
	(*func).push_back(new ASSIGNtoy(L"TICKCOUNT"));
	(*func).push_back(new MACROtoy());

	context.heap.back()->func_offset = (*func).size();
	
	std::vector<toyCODE*>	*subs;
	subs = &(context.heap.back()->subs);

	(*subs).push_back(new IFcode());
	(*subs).push_back(new LOOPcode());
	(*subs).push_back(new MACHINEtoy());
	context.heap.back()->subs_offset = (*subs).size();

	MACHINEignore = context.getsubpos(L"@");
}

void toyMACHINE::prepare() {
	context.prepare();
	//context.func_offset = context.func.size();
	//context.subs_offset = context.subs.size();
}

int64_t *toyMACHINE::getdollar() {
	return context.getdollar();
}
void toyMACHINE::do_txt_convert(std::wstring src) {
	int	k = 0;
	int64_t	a = 0, b = 0;
	int64_t	c = 0, d = 0;
	bool	invert = false;
	bool	b_mode = false;
	for(int i = 0; i < src.size(); i++) {
		switch(k) {
		case 0:
			if(iswspace(src[i])) i++;

			if(src[i] == L'-') {
				invert = true;
			} else
		case 1:
			if(iswdigit(src[i])) {
				k = 1;
				a *= 10;
				a += src[i]-L'0';
				c++;
			} else if(src[i] == L'.') {
				k = 2;
				b_mode = true;
			} else {
				return;
			}
			break;
		case 2:
			if(iswdigit(src[i])) {
				b *= 10;
				b += src[i]-L'0';
				d++;
			} else {
				return;
			}
			break;
		}
	}
	tsbcode	cd = invalidtsb;
	if(b_mode) {
		double	dec;
		dec = b;
		while(d--) dec /= 10.0;
		dec += a;
		if(invert) dec = -dec;
		cd = context.allocanondec(dec);
	} else {
		int64_t	num;
		num = a;
		if(invert) num = -num;
		cd = context.allocanonnum(num);
	}
	context.codepush(cd);
}

int toyMACHINE::do_asgn_parm(const wchar_t *src, int sz, toyCODE *cT, bool is_code) {
	int	ret = -1;
	std::wstring	txt, txta;
		
	for(int i = 0; i < sz; i++) {
		switch(src[i]) {
		case L',':
		case L'"':
			ret = i;
			break;
		case L'[': case L'<':
			ret = i;
			if(txta.size()>0 && txt.size()==0) {
				txt = txta;
				txta = L"";
			}
			break;
		}
		if(ret != -1) break;
		if(iswspace(src[i])) {
			if(txta.size()==0) {
				txta = txt;
				txt = L"";
			} else {
				ret = i;
			}
		} else {
			txt += src[i];
		}
	}
	if(ret == -1) ret = sz;
	tsbcode	cd;
	if(is_code) {
		if(txta.size()>0) {
			cd = context.resolvevar(txta);
			if(cd != invalidtsb) {
				context.codepush(cd);
				cd = context.resolvevar(txt);
				if(cd == invalidtsb)
					do_txt_convert(txt);
				else
					context.codepush(cd);
			}
		}
		return ret;
	} else if(cT != nullptr) {
		if(txta.size()>0) {
			cd = cT->sheap.getcorrect(txta);
			if(cd != invalidtsb) {
				context.codepush(cd);
				txta = L"";
			}
		}
	}
	if(txta.size()>0) {
		cd = context.getfuncpos(txta);
		if(cd != invalidtsb) {
			context.codepush(cd);
			cd = context.resolvevar(txt);
			if(cd == invalidtsb)
				do_txt_convert(txt);
			else
				context.codepush(cd);
		}
	} else if(txt.size()>0) {
		tsbcode cd = context.resolvevar(txt);
		if(cd == invalidtsb) {
			do_txt_convert(txt);
		} else
		context.codepush(cd);
	}
	return ret;
}

int toyMACHINE::do_text_parm(const wchar_t *src, int sz, toyCODE *cT) {
	std::wstring	txt;
	wchar_t			sk = 0;
	for(int i = 0; i < sz; i++) {
		switch(src[i]) {
		case L't':
			if(sk == L'\\') {
				txt += L'\t';
			} else {
				txt += L't';
			}
			break;
		case L'n':
			if(sk == L'\\') {
				txt += L'\n';
			} else {
				txt += L'n';
			}
			break;
		case L'"':
			if(sk == L'\\') {
				txt += src[i];
			} else if(i > 0) {
				tsbcode	cd = context.allocanonstr(txt);
				context.codepush(cd);
				return i+1;
			}
			break;
		case L'\\':
			if(sk == L'\\') {
				txt += L'\\';
				sk = 1;
			}
			break;
		default:
			txt += src[i];
		}
		if(sk == 1)
			sk = 0;
		else
			sk = src[i];
	}

	return 0;
}

bool toyMACHINE::acceptfuncchar(std::wstring src, int pos, int sz) {
	if(iswspace(src[pos])) return false;

	if(src[pos] == L'<' || src[pos] == L'[') {
		wchar_t	sk = src[pos], sk2 = L']';
		if(src[pos] == L'<') sk2 = L'>';

		bool skok = false; int	skn = 0;
		for(int i = pos; i < src.size(); i++) {
			if(src[i] == sk) skn++;
			if(src[i] == sk2) {
				skok = true;
				break;
			}
		}

		if(skok == true) {
			if(skn == 1) return false;
		} else {
			if(sk == L'[') return false;
			if(sk == L'<' && sz > 0) {
				if(src[pos-sz] != L'<') return false;
			}
		}
		return true;
	}
	if(sz > 0) {
		//if(iswalpha(src[pos]) || src[pos] == L'_') {
			switch(src[pos-1]) {
			case L'[': case L']': 
			case L'<': case L'>': case L'=': case L'-': case L'+':
			case L'*': case L'/': case L'%':
				if(iswalnum(src[pos])||src[pos]==L'_'||src[pos]==L'$')
					return false;
			}
		//}
	}
	if(src[pos] == L'"' || src[pos] == L'-')
		return false;
	return true;
}
bool toyMACHINE::acceptasgnchar(std::wstring src, int pos, int sz) {
	if(iswspace(src[pos])) return false;
	if(sz == 0) {
		if(iswdigit(src[pos])) return false;
	}
	switch(src[pos]) {
	case L'<': case L'[': case L'>': case L']':
		return false;
	}
	return true;
}

std::wstring toyMACHINE::getfunc(std::wstring src, int *pos) {
	std::wstring ret;
	int	k = 0;
	int i;
	for(i = 0; i < src.size(); i++) {
		if(iswspace(src[i])) {
			if(ret.size() > 0) {
				break;
			}
		} else {
			if(k == 0) {
				if(ret.size()==0 && src[i] == L'/') {
					ret += src[i];
					k = 1;
				} else if(ret.size()==0 && (src[i] == L'@' || src[i] == L'©')) {
					ret = L"@";
					i = src.size();
					break;
				} else if(acceptfuncchar(src, i, ret.size())) {
					ret += src[i];
					k = 2;
				} else break;
			} else if(k == 1) {
				ret += src[i];
			} else if(k == 2) {
				if(acceptfuncchar(src, i, ret.size())) {
					ret += src[i];
				} else break;
			}
		}
	}
	*pos = i;
	return ret;
}
std::wstring toyMACHINE::getasgn(std::wstring src, int *pos) {
	std::wstring	ret;
	int i;
	wchar_t	sk, sk2;
	bool sk0 = false, sk1 = false;;
	for(i = *pos; i < src.size(); i++) {
		if(iswspace(src[i])) {
			if(ret.size() > 0) {
				break;
			}
		} else if(sk0 == false) {
			sk = src[i];
			sk0 = true;
			if(sk == L'<') sk2 = L'>';
			else if(sk == L'[') sk2 = L']';
			else {sk0 = false; break; }
			ret += sk;
		} else if(acceptasgnchar(src, i, ret.size())) {
			ret += src[i];
		} else if(src[i] == sk2) {
			ret += src[i];
			sk1 = true;
			break;
		} else {
			*pos = src.size();
			return L"";
		}
	}
	if(sk1 == false) {
		return L"";
	}
	*pos = i+1;
	return ret;
}
std::wstring toyMACHINE::getparm(std::wstring src, int *pos) {
	std::wstring	ret;
	int	i; int k = 0; wchar_t	pk = 0;
	for(i = *pos; i < src.size(); i++) {
		switch(k) {
		case 0:
			if(iswspace(src[i])==0) {
				ret += src[i];
			}
		case 1:
			k = 2;
			if(src[i] == L'"') {
				k = 3;
			}
			break;
		case 2:
			if(iswspace(src[i])) {
				if(ret.size()>0) {
					wchar_t	sk, sl = 0;
					sk = ret[ret.size()-1];
					if(i < src.size()-1) {
						sl = src[i+1];
					}
					if(iswspace(sk)==0) {
						switch(sk) {
						case L'"': case L',':
							break;
						default:
							if(sl != L',') {
								ret += src[i];
							}
						}
					}
				}
				k = 0;
			} else if(src[i] == L'"') {
				k = 3;
				ret += src[i];
			} else {
				ret += src[i];
			}
			break;
		case 3:
			if(pk == L'\\') {
				ret += src[i];
				pk = 0;
			} else {
				pk = src[i];
				ret += src[i];
				if(pk == L'"') {
					pk = 0;
					k = 0;
				}
			}
			break;
		}
	}
	*pos = i+1;
	for(int i = ret.size(); i > 0; i--) {
		if(iswspace(ret[i-1])==0) {
			ret.resize(i);
			break;
		}
	}
	return ret;
}

bool toyMACHINE::ok_wait_exec(toyFUNC *fn, int cpo) {
	int it = cpo;
	uint32_t	msg = context.code[it] & tsbcmsg;
	uint32_t	bco = context.code[it] &~tsbcmsg;
	msg >>= 32UL-4UL;
	std::wstring	*str = nullptr;
	if(msg == ts_stop) {
		it++;
		bco--;
		if(bco) {
			if(context.findtoy(context.code[it])==fn) {
				it++;
				bco--;
				if(bco) {
					toyHEAP *thp = nullptr;
					tsbcode tbc = invalidtsb;
					str = context.findstr(context.code[it], it-2, ts_ends, false, &thp, &tbc);
					if(str) {
						toyIMMEDIAT	*ti = new toyIMMEDIAT();
						ti->backfunc = context.code[it-1];
						context.code[it-1] = context.getsubpos(L"@");
						ti->parent = context.heap.back();
						ti->func_offset = ti->parent->func_offset;
						ti->subs_offset = ti->parent->subs_offset;
						for(int i = 0; i < 9; i++) {
							ti->offsets[i] = ti->parent->offsets[i];
						}
						ti->mac = this;
						ti->code_position = context.code_position;
						toyPAIR	*fnd = nullptr, *pfnd = nullptr;
						fnd = context.imme;
						while(fnd) {
							if( fnd->okc == tbc && fnd->othp == thp) {
								break;
							}
							pfnd = fnd;
							fnd = fnd->next;
						}
						if(fnd) {
							ti->prev = fnd->ti;
							fnd->ti = ti;
						} else {
							fnd = new toyPAIR();
							fnd->okc = tbc;
							fnd->othp = ti->parent;
							fnd->code_position = cpo;
							fnd->ti = ti;
							fnd->next = context.imme;
							context.imme = fnd;
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}

int toyMACHINE::feedline(std::wstring src, std::wstring stop2, std::wstring *stop3) {
	int	pos = 0;

	bool	can_continue_code = false;
	bool	asgnbool = false;

	toyFUNC	*fn = nullptr;
	toyCODE	*cd = nullptr;
	bool	is_code = false;
	bool	known_p = false;
	std::wstring	func = getfunc(src, &pos);
	std::wstring	asgn = getasgn(src, &pos);
	std::wstring	parm = getparm(src, &pos);

	if(func.size() == 0) return -6;
	if(stop2.size()==0) {
		if(func == L"/]" || func == L"/>") return -2;
	} else {
		//stop2 += func[func.size()-1];
		if(func == stop2) return -2;
	}
	if(metarget) return -13;
	if(func == L"@") return context.code_position;
	if(asgn.size()>0) if(asgn[0]==L'<') known_p = true;

	size_t	code_position_old = context.code.size();
	context.code_position = context.code.size();

	context.codepush(mktsbc(0, tsbtype::ts_stop));
	tsbcode	funcpos = context.getfuncpos(func);
	if(funcpos == invalidtsb) {
		funcpos = context.getsubpos(func);
		cd = context.findsub(funcpos);
		if(cd) is_code = cd->is_code;
	} else {
		fn = context.findtoy(funcpos);
		if(fn) is_code = fn->is_code;
	}
	if(funcpos != invalidtsb) {
		if((cd)  || (fn && (is_code == false||fn->target_me == true))) {
			if(bank.size()>0 && bank.back()->cant_just == true) {
				context.codepush(context.getsubpos(L"@"));
			} else {
				context.codepush(funcpos);
			}
		} else {

			context.codepush(context.getsubpos(L"@"));
		}
		tsbcode asgnpos = invalidtsb;
		if(fn && fn->can_var) {
			if(asgn.size()>0)
				asgnpos = context.getasgnpos(asgn, known_p);
			if(asgnpos != invalidtsb) {
				context.codepush(asgnpos);
			} else if(asgn.size()>0) {
				tsbcode	cd = context.allocanonstr(asgn);
				context.codepush(cd);
			} else {
				context.codepush(mktsbc(0, ts_cash));
			}
		} else {
			if(asgn.size()>0) asgnpos = context.resolvevar(asgn);
			if(asgnpos == invalidtsb) asgnpos = mktsbc(0, ts_cash);
			context.codepush(asgnpos);
		}
		asgnbool = false;
		if(fn) asgnbool = is_code;
		if(fn && fn->target_me) asgnbool = false;
		int	lp = 0, pp = 0, cp = 0;
		do {
			lp = parm.size()-cp;
			if(lp) {
				wchar_t	scp = L']';
				switch(parm[cp]) {
				case L',':	cp++; break;
				case L'<': scp = L'>'; case L'[':
					if(is_code) {
						cp++;
						*stop3 = L"/";
						if(fn && fn->target_me) metarget = fn;
						context.codepush(mktsbc(0, tsbtype::ts_ends));
						while(cp<parm.size()) {
							
							if(parm[cp]==L'/') cp = parm.size();
							else *stop3 += parm[cp++];
						}
						*stop3 += scp;
						can_continue_code = true;
					}
					break;
				case L'"':	cp += do_text_parm(&parm[cp], parm.size()-cp, cd); break;
				default:	cp += do_asgn_parm(&parm[cp], parm.size()-cp, cd, asgnbool); break;
				}
			}
			pp = parm.size()-cp;
		} while(lp != pp);
		if(cp < parm.size()) {
			int	hmte = context.code.size()-code_position_old;
			while(hmte-- > 0) context.code.pop_back();
			context.code_position = code_position_old;
			return -__LINE__;
		}
	} else {
		int	hmte = context.code.size()-code_position_old;
		while(hmte-- > 0) context.code.pop_back();
		context.code_position = code_position_old;
		return -__LINE__;
	}
	if(context.code.size()<=context.code_position) {
		return -__LINE__;
	}
	context.code[context.code_position] = mktsbc(context.code.size()-context.code_position, tsbtype::ts_stop);
	bool can_just = true;
	tsbcode	djcode = invalidtsb;
	if(bank.size()>0 && bank.back()->cant_just == true) can_just = false;
	if(fn && fn->just_auto && can_just) {
		int	heap1, heap2;
		int	code1, code2;
		heap1 = context.heap.size();
		code1 = context.heap.back()->subs.size();
		if((djcode = fn->do_just(&this->context, context.code_position, context.code.size()-context.code_position))!=invalidtsb) {
			heap2 = context.heap.size();
			if(heap2-heap1 > 0) {
				code2 = context.heap[heap1-1]->subs.size();
				if(code2-code1 > 0) {
					bank.push_back(context.heap[heap1-1]->subs.back());
				}
			}
			can_continue_code = true;
		}
	}
	if(fn && fn->auto_exec) {
		if(asgn.size()>0) {
			if(asgn[0] == L'[') {
				if(ok_wait_exec(fn, code_position_old)) {
					if(can_continue_code) return -1;
					return context.code_position;
				}
			}
		}
		if(djcode != invalidtsb) {
			context.code[context.code_position+1] = djcode;
		}
		if(can_continue_code) return -666001;
		return -666000;
	}
	
	if(can_continue_code) return -1;
	return context.code_position;
}
int toyMACHINE::execsolo(int it, size_t csz) {
	tsbcode		cd = context.code[it];
	uint32_t	msg = cd & tsbcmsg;
	uint32_t	bco = cd &~tsbcmsg;
	msg >>= 32UL-4UL;
	int s = bco;
	if(s <= csz && msg == tsbtype::ts_stop) {
		toyFUNC	*fn; toyCODE *co;
		cd = context.code[it+1];
		msg = cd & tsbcmsg;
		bco = cd &~tsbcmsg;
		msg >>= 32UL-4UL;
		tsbcode	*cp = nullptr;
		if(msg == tsbtype::ts_func) {
			fn = context.findtoy(cd);
			if(fn != nullptr) {
				fn->func(&context, it+1, s-1);
				return s;
			}
		} else if(msg == tsbtype::ts_code) {
			co = context.findsub(cd);
			if(co != nullptr) {

				co->code(this, it, s);
				return s;
			}
		}
	}
	return 0;
}
int toyMACHINE::execcode(int it, size_t csz) {
	size_t	lp = 0, pp = 0, cp = 0;
	do {
		lp = csz-cp;
		if(lp) {
			cp += execsolo(cp+it, csz-cp);
		}
		pp = csz-cp;
	} while(lp != pp);
	return cp;
}
int toyMACHINE::execline(int err) {
	if(err >= 0 && err < context.code.size()) {
		tsbcode		cd = context.code[err];
		uint32_t	msg = cd & tsbcmsg;
		uint32_t	bco = cd &~tsbcmsg;
		msg >>= 32UL-4UL;
		int s = bco;
		if((err+s) <= context.code.size() && msg == tsbtype::ts_stop) {
			return execsolo(err, s);
			return s;
		}
	}
	return -__LINE__;
}


int IFcode::code(toyMACHINE *mac, int it, size_t csz) {
	int64_t	*cond = mac->getdollar();
	if(cond && *cond != 0) {
		*cond = 0;
		for(int i = it; i < it+csz; i++) {
			uint32_t	msg = mac->context.code[i] & tsbcmsg;
			uint32_t	bco = mac->context.code[i] &~tsbcmsg;
			msg >>= 32UL-4UL;
			if(msg == ts_ends) {
				return mac->execcode(i+1, csz-((i-it)+1));
			}
		}
	} else if (cond) {
		*cond = 1;
	}
	return 0;
}

int LOOPcode::code(toyMACHINE *mac, int it, size_t csz) {
	int64_t	*cond = nullptr;
	int	u = 0;
	for(u = it; u< it+csz; u++) {
		uint32_t	msg = mac->context.code[u] & tsbcmsg;
		uint32_t	bco = mac->context.code[u] &~tsbcmsg;
		msg >>= 32UL-4UL;
		
		if(mac->context.code[u] != mktsbc(0, ts_cash))
			cond = mac->context.findnum(mac->context.code[u]);

		if(cond || msg == ts_ends) {
			break;
		}
	}
	if(cond == nullptr) cond = mac->getdollar();
	if(cond && *cond != 0) {
		//*cond = 0;
		for(int i = u; i < it+csz; i++) {
			uint32_t	msg = mac->context.code[i] & tsbcmsg;
			uint32_t	bco = mac->context.code[i] &~tsbcmsg;
			msg >>= 32UL-4UL;
			if(msg == ts_ends) {
				int	ret = 0;
				while((*cond) > 0) {
					
					ret = mac->execcode(i+1, csz-((i-it)+1));
					*cond = (*cond)-1;
				}
				return ret;
			}
		}
	}
	return 0;
}

int toyMAKER::make2(std::wstring toy, toyMACHINE *mac) {
	int	err; bool	auto_exec = false;
	if(target == nullptr) target = mac;
	std::wstring	stop2 = L"";
	int		heapsize1, heapsize2;
	heapsize1 = mac->context.heap.size();
	if(code_labels.size()>0)
		err = target->feedline(toy, code_labels.back(), &stop2);
	else
		err = target->feedline(toy, stop2, &stop2);
	heapsize2 = mac->context.heap.size();
	if(err <= -666000) auto_exec = true;
	if(err == -666001 || err == -1) {
		code_subs.push_back(mac->context.code_position);
		heaps.push_back(heapsize2-heapsize1);
		std::wstring	stop3 = L"";
		if(stop2.size()) {
			stop3 = L"";
			stop3 += stop2;
			//stop3.resize(stop3.size()-1);
		}
		code_labels.push_back(stop3);
	} else if(err == -2) {
		if(code_subs.size()>0) {
			if(mac->metarget != nullptr) {
				target->context.codepush(target->context.allocanonstr(targetstr));
				target->metarget = nullptr;
				targetstr = L"";
			}
			uint32_t	m2sz;
			target->context.code[err = code_subs.back()] = mktsbc((m2sz=(target->context.code.size()-code_subs.back())), tsbtype::ts_stop);
			
			code_subs.pop_back();
			code_labels.pop_back();
			int	heappos = heaps.back();
			if(heappos > 0) {
				mac->context.heap.resize(mac->context.heap.size()-heappos);
				if(mac->bank.size()>0) {
					toyCODE	*bc = mac->bank.back();
					bc->sz = m2sz;
					for(int i = 0; i < 9; i++) {
						bc->immcon.offsets[i] = mac->context.heap.back()->offsets[i];
					}
					bc->immcon.imme = mac->context.imme;
					if(mac->context.isso.size()>0) {
						mac->context.imme = mac->context.isso.back();
						mac->context.isso.pop_back();
					}
					//mac->context.heap.back()->subs_offset++;
					
					mac->bank.pop_back();
				}
			}
			heaps.pop_back();
			err = -2;
		}
	} else if(err == -6) {
		if(target->metarget) {
			targetstr += L" \n";
		}
	} else if(err == -13) {
		targetstr += toy + L"\n";
	}
	if(mac->context.heap.size()>1 || code_subs.size()>0) {
		auto_exec = false;
	}
	if(auto_exec) {
		exec(exec_pos);
		exec_pos = mac->context.code.size();
		err = exec_pos;
	}
	if(code_subs.size()>0||heaps.size()>0) return -3;
	if(err >= 0) return err;
	if(err == -6) return -1;
	return -__LINE__;
}
int toyMAKER::make(std::wstring toy, toyMACHINE *mac) {
	std::wstring	atoy;
	bool	ok_do_it = false;
	int		dodo_not = 0;
	int		ret = mac->context.code.size();
	int		ret2 = ret;
	
	if(target && target->metarget != nullptr) {
		return make2(toy, mac);
	}
	
	for(int i = 0; i < toy.size(); i++) {
		if(toy[i] == L'"') {
			if(dodo_not == 0) dodo_not = 2;
			if(dodo_not) dodo_not--;
		}
		if(toy[i] == L'\\') {
			if(i+1 < toy.size()) {
				dodo_not++;
			}
		} else if(dodo_not>1) {
			dodo_not--;
		}
		if(toy[i] == L'|') {
			if(dodo_not <= 0)
				ok_do_it = true;
		}
		if(ok_do_it) {
			if(atoy.size()>0)
				if((ret2 = make2(atoy, mac))<-10) 
					return -__LINE__;
			atoy = L"";
			ok_do_it = false;
		} else {
			atoy += toy[i];
		}
	}
	if(atoy.size()>0) 
		if((ret2=make2(atoy, mac))>=0) {
			return ret2;
		}
	if(ret < mac->context.code.size()) {
		return ret2;
	}
	return -8;
}

void toyMAKER::exec(int perr) {
	if(target) {
		if(target->context.code_position == 0) target->prepare();
		int	err = 1;
		int pos = perr;
		while(err > 0) {
			//std::wcout << err << std::endl;
			err = target->execline(pos);
			if(err > 0) target->context.code_position += err;
			pos += err;
		}

		//target->execline(err);
	}
}

MACROtoy::MACROtoy() {
	name = L"macro";
	can_var = true;
	is_code = true;
	just_auto = true;
}

tsbcode MACROtoy::do_just(toyCONTEXT *ctx, int it, size_t csz) {
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
					tsbtype				prudence = tsbtype::ts_protect;
					if(vnam[0] == L'[') prudence = tsbtype::ts_private;
					cvnam.assign(vnam.c_str()+1, vnam.size()-2);
					MACROcode	*mc = new MACROcode();
					mc->prudence = prudence;
					mc->name = cvnam;
					mc->startpos = it;
					mc->sz = 0;
					tsbcode nc = ctx->alloccode(mc);

					return nc;
				}
			}
		}
	}
	return invalidtsb;
}

int MACROtoy::func(toyCONTEXT *ctx, int it, size_t csz) {
	
	return __LINE__;
		
}



MACROcode::MACROcode() {
	startpos = sz = 0;
	is_code = false;
	
	name = L"macro";
}

int MACROcode::code(toyMACHINE *mac, int it, size_t csz) {
	toyHEAP		hipo(sheap);
	int	ret = 0;
	int64_t		*reti = nullptr, *retia = nullptr;
	double		*retd = nullptr, *retda = nullptr;
	std::wstring	*rets = nullptr, *retsa = nullptr;
	for(int i = it; i < it+csz; i++) {
		uint32_t	msg = mac->context.code[i] & tsbcmsg;
		uint32_t	bco = mac->context.code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		//if(msg == ts_stop) break;
		if(i == it+2) {
			if(msg != ts_cash) {
				switch(msg) {
				case ts_num:
				case ts_pnum:
					for(int u = 0; u < hipo.pnum.vars.size(); u++) {
						if(hipo.pnum.names[u].size()>0) {
							if(hipo.pnum.names[u][0] == L'(') {
								retia = mac->context.findnum(mac->context.code[i]);
								reti = &hipo.pnum.vars[u];
								u = hipo.pnum.vars.size();
							}
						}
					}
					break;
				case ts_dec:
				case ts_pdec:
					for(int u = 0; u < hipo.pdec.vars.size(); u++) {
						if(hipo.pdec.names[u].size()>0) {
							if(hipo.pdec.names[u][0] == L'(') {
								retda = mac->context.finddec(mac->context.code[i]);
								retd = &hipo.pdec.vars[u];
								u = hipo.pdec.vars.size();
							}
						}
					}
					break;
				case ts_txt:
				case ts_ptxt:
					for(int u = 0; u < hipo.ptxt.vars.size(); u++) {
						if(hipo.ptxt.names[u].size()>0) {
							if(hipo.ptxt.names[u][0] == L'(') {
								retsa = mac->context.findstr(mac->context.code[i], it);
								rets = &hipo.ptxt.vars[u];
								u = hipo.ptxt.vars.size();
							}
						}
					}
					break;
				}
			}
		}
		if(i > it+2 && (i+1) < it+csz) {
			if(msg == ts_pnum || msg == ts_pdec || msg == ts_ptxt) {
				if(bco < hipo.offsets[msg]) {
					int64_t			*ppn;
					double			*ppd;
					std::wstring	*ppt;
					tsbcode			mcc = mac->context.code[i+1];
					uint32_t	msg2 = mac->context.code[i+1] & tsbcmsg;
					uint32_t	bco2 = mac->context.code[i+1] &~tsbcmsg;
					msg2 >>= 32UL-4UL;
					switch(msg) {
					case ts_pnum:
						ppn = &hipo.pnum.vars[bco];
						if(ppn) {
							int64_t	*pppn = mac->context.findnum(mcc);
							if(pppn) *ppn = *pppn;
						}
						break;
					case ts_pdec:
						ppd = &hipo.pdec.vars[bco];
						if(ppd) {
							double	*pppd = mac->context.finddec(mcc);
							if(pppd) *ppd = *pppd;
						}
						break;
					case ts_ptxt:
						ppt = &hipo.ptxt.vars[bco];
						if(ppt) {

							std::wstring	*pppt = mac->context.findstr(mcc, it);
							if(pppt) *ppt = *pppt;
						}
						break;
					}
				}
			}
			i++;
		}
	}
	
	uint32_t	coffsets[9];
	uint32_t	cfunc_offset;
	uint32_t	csubs_offset;

	toyHEAP		*hp = immcon.parent;
	for(int i = 0; i < 9; i++) {
		coffsets[i] = hp->offsets[i];
		hp->offsets[i] = immcon.offsets[i];
	}
	cfunc_offset = hp->func_offset;
	hp->func_offset = immcon.func_offset;
	csubs_offset = hp->subs_offset;
	hp->subs_offset = immcon.subs_offset;

	std::vector	<toyHEAP*>	backed;
	int	disto = -1;
	for(int i = 0; i < mac->context.heap.size(); i++) {
		if(mac->context.heap[i] == immcon.parent) {
			disto = i+1;
			break;
		}
	}
	if(disto != -1) {
		for(int i = mac->context.heap.size(); i > disto; i--) {
			backed.push_back(mac->context.heap.back());
			mac->context.heap.pop_back();
		}
	}
	mac->context.heap.push_back(&hipo);

	mac->context.isso.push_back(mac->context.imme);
	mac->context.imme = immcon.imme;

	for(int i = startpos+4; i < startpos+sz; i++) {
		uint32_t	msg = mac->context.code[i] & tsbcmsg;
		uint32_t	bco = mac->context.code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == ts_stop) {
			if(bco == 0) bco = 1;
			mac->execcode(i, bco);
			i += bco-1;
		}
	}
	immcon.imme = mac->context.imme;
	mac->context.imme = mac->context.isso.back();
	mac->context.isso.pop_back();

	mac->context.heap.pop_back();

	while(backed.size()) {
		mac->context.heap.push_back(backed.back());
		backed.pop_back();
	}
	
	hp->func_offset = cfunc_offset;
	hp->subs_offset = csubs_offset;
	for(int i = 0; i < 9; i++) {
		hp->offsets[i] = coffsets[i];
	}

	if(reti && retia) *retia = *reti;
	if(retd && retda) *retda = *retd;
	if(rets && retsa) {
		*retsa = *rets;
	}
	return sz;
}

