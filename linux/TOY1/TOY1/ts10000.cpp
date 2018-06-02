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

bool toyDATA::finddata(tsbcode cd, toyTOY *rv) {
	bool	ret = false;

	return ret;
}

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
		for(int i = (int)ctx->heap.size(); i > disto; i--) {
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
//	int	ret;
	for(int i = offset; i > 0; i--) {
		if(names[i-1].compare(src)==0) {
			return i-1;
		}
	}
	return -1;
}

template <typename T>
int toyVARS<T>::getpospre(std::wstring src, int offset) {
	for(int i = offset; i > 0; i--) {
		if(names[i-1].size() <= src.size()) {
			if(names[i-1].compare(0, names[i-1].size(), src)==0) {
				return i-1;
			}
		}
	}
	return -1;
}


bool toyDATA::fillcode(std::wstring src, int pos, toySTREAM *rv) {
	return false;
}

bool toyDATA::fillcodereverse(std::wstring src, int pos, toySTREAM *rv) {
	int count_num = 0, count_dec = 0, count_txt = 0, count_data = 0;

	if(pos < 0) return false;
	if(pos >= src.size()) return false;




	for(int u = objs.size(); u > 0; u--) {
		toyOBJECT	*obj = objs[u-1];
		if(obj) {
			for(int i = obj->integerL.size(); i > 0; i--) {
				if(src.compare(pos, src.size()-pos, obj->integerL[i-1])==0) {
					rv->code.push_back(mktsbc(superindex-(count_num+(i-1)), ts_num));
					return true;
				}
			}
			count_num += obj->integerL.size();

			for(int i = obj->decimalL.size(); i > 0; i--) {
				if(src.compare(pos, src.size()-pos, obj->decimalL[i-1])==0) {
					rv->code.push_back(mktsbc(superindex-(count_dec+(i-1)), ts_dec));
					return true;
				}
			}
			count_dec += obj->textL.size();

			for(int i = obj->textL.size(); i > 0; i--) {
				if(src.compare(pos, src.size()-pos, obj->textL[i-1])==0) {
					rv->code.push_back(mktsbc(superindex-(count_txt+(i-1)), ts_txt));
					return true;
				}
			}
			count_txt += obj->textL.size();

			for(int i = obj->dataL.size(); i > 0; i--) {
				if(src.compare(pos, src.size()-pos, obj->dataL[i-1])==0) {
					return data[i-1]->fillcodereverse(src, pos+obj->dataL[i-1].size(), rv);
				}
			}
			count_data += obj->dataL.size();
		}
	}
	return false;
}

bool toyDATALIST::getcode(std::wstring src, toySTREAM *rv) {
	toyDATA	*dt = nullptr;
	int dtindex; int pos;
	dtindex = data.getpospre(src, offset);
	if(dtindex >= 0) {
		dt = data.vars[dtindex];
		pos = data.names[dtindex].size();
	}

	if(dt) {
		rv->code.push_back(mktsbc(superindex-dtindex, ts_cash));

		if(dt->fillcodereverse(src, pos, rv)) {
			return true;
		}
		if(dt->fillcode(src, pos, rv)) {
			return true;
		}
	}

	return false;
}

toyDATALIST::toyDATALIST() {
	offset = 0;
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

toyHEAP::~toyHEAP() {
	for(int i = 0; i < subs.size(); i++) {
		if(subs[i]) delete subs[i];
	}
	for(int i = 0; i < func.size(); i++) {
		if(func[i]) delete func[i];
	}
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
	heap.push_back(&baseheap);
}

toyCONTEXT::~toyCONTEXT() {
	for(int i = 0; i < isso.size(); i++) {
		toyPAIR	*a, *b;
		a = isso[i];
		while(a) {
			b = a;
			a = a->next;

			toyIMMEDIAT	*c, *d;
			c = b->ti;
			while(c) {
				d = c;
				c = c->prev;

				delete d;
			}

			delete b;
		}
	}
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
	if(cd == mktsbc(0, ts_cash)) {
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
	//tsbcode	ret;
	uint32_t	offset = 0, offset2 = 0;
	for(int i = (int)heap.size(); i > 0; i--) {
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
	//tsbcode	ret;
	uint32_t	offset = 0, offset2 = 0;
	for(int i = (int)heap.size(); i > 0; i--) {
		for(int u = (int)heap[i-1]->func.size(); u > 0; u--) {
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
	for(int i = (int)heap.size(); i > 0; i--) {
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
			hp->offsets[msg] = (uint32_t)hp->pnum.vars.size();
		} else if(msg == tsbtype::ts_num) {
			ret = mktsbc(hp->num.vars.size(), msg);
			hp->num.names.push_back(vnam);
			hp->num.vars.push_back(src);
			hp->offsets[msg] = (uint32_t)hp->num.vars.size();
		}
	}
	return ret;
}

tsbcode toyCONTEXT::alloccode(toyCODE *mc) {
	tsbcode ret = invalidtsb;

	if(heap.size()>0) {
		toyHEAP	*hp = heap[heap.size()-1];
		uint32_t	acsz = 0;
		for(int i = (int)heap.size(); i > 0; i--) {
			acsz += (uint32_t)heap[i-1]->subs.size();
		}
		
		ret = mktsbc(hp->subs.size(), ts_code);
		hp->subs.push_back(mc);
		hp->subs_offset = (uint32_t)hp->subs.size();
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

	if(ctx->code[it] == mktsbc(0, ts_cash)) {
		cip = ctx->getdollar();
		isdollar = true;
		return ts_num;
	} else if(msg == ts_num || msg == ts_pnum) {
		cip = ctx->findnum(ctx->code[it-1], (tsbtype)msg);
		if(cip) return ts_num;
	} else if(msg == ts_dec || msg == ts_pdec) {
		cid = ctx->finddec(ctx->code[it-1], (tsbtype)msg);
		if(cid) return ts_dec;
	} else if(msg == ts_txt || msg == ts_ptxt) {
		cis = ctx->findstr(ctx->code[it-1], (tsbtype)msg);
		if(cis) return ts_txt;
	}
	return ts_stop;
}

tsbtype cgame::msgtotype(uint32_t msg) {
	if(msg == ts_cash) {
		return ts_num;
	} else if(msg == ts_num || msg == ts_pnum || msg == ts_anum) {
		return ts_num;
	} else if(msg == ts_dec || msg == ts_pdec || msg == ts_adec) {
		return ts_dec;
	} else if(msg == ts_txt || msg == ts_ptxt || msg == ts_atxt) {
		return ts_txt;
	}
	return ts_stop;
}

tsbtype cgame::what() {
	tsbtype		ret = ts_stop;
	uint32_t	msg, bco;
	msg = ctx->code[it] & tsbcmsg;
	bco = ctx->code[it] &~tsbcmsg;
	msg >>= 32UL-4UL;

	ret = msgtotype(msg);
	if(ret != invalidtsb) {
		return ret;
	} else if(msg == ts_func) {
		conv = ctx->findtoy(ctx->code[it]);
		it++; csz--;
		if(domore()) {
			if(conv) {
				if(conv->okconvert(ctx->code[it])) {
					return ts_txt;
				}
			}
		}
		conv = nullptr;
		ret = msgtotype((ctx->code[it] & tsbcmsg) >> (32UL-4UL));
		if(ret != invalidtsb) return ret;
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
	std::wstring *ret = nullptr;
	if(conv) {
		ret = &convret;
		convret = conv->convert(ctx, ctx->code[it], it);
		conv = nullptr;
	}
	if(ret == nullptr) {
		ret = ctx->findstr(ctx->code[it], it);
	}
	if(ret) {
		it++; csz--;
	}
	return ret;
}

int64_t *cgame::getinteger() {
	int64_t *ret;
	ret = ctx->findnum(ctx->code[it]);
	if(ret) {
		it++; csz--;
	}
	return ret;
}

double *cgame::getdecimal() {
	double *ret;
	ret = ctx->finddec(ctx->code[it]);
	if(ret) {
		it++; csz--;
	}
	return ret;
}

void cgame::ignoreone() {
	it++; csz--;
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
	(*func).push_back(new COMPAREtoy());
	//(*func).push_back(new ASSIGNtoy(L"=="));
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

toyMACHINE::~toyMACHINE() {
	for(int i = 1; i < context.heap.size(); i++) {

	}
}

void toyMACHINE::prepare() {
	context.prepare();
	//context.func_offset = context.func.size();
	//context.subs_offset = context.subs.size();
}

int64_t *toyMACHINE::getdollar() {
	return context.getdollar();
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
	int64_t	varcon = 0;
	int64_t	*cond = nullptr;
	int	u = 0;
	for(u = it; u< it+csz; u++) {
		uint32_t	msg = mac->context.code[u] & tsbcmsg;
		uint32_t	bco = mac->context.code[u] &~tsbcmsg;
		msg >>= 32UL-4UL;
		
		if(mac->context.code[u] != mktsbc(0, ts_cash)) {
			cond = mac->context.findnum(mac->context.code[u]);
			if(msg == ts_anum) {
				if(cond) {
					varcon = *cond;
					cond = &varcon;
				}
			}
		}
		if(cond || msg == ts_ends) {
			break;
		}
	}
	if(cond == nullptr) cond = mac->getdollar();
	if(cond) varcon = *cond;

	if(cond && *cond > 0) {
		//*cond = 0;
		for(int i = u; i < it+csz; i++) {
			uint32_t	msg = mac->context.code[i] & tsbcmsg;
			uint32_t	bco = mac->context.code[i] &~tsbcmsg;
			msg >>= 32UL-4UL;
			if(msg == ts_ends) {
				int	ret = 0;
				while(*cond > 0) {
					
					ret = mac->execcode(i+1, csz-((i-it)+1));
					*cond = (*cond) - 1;

				}
				return ret;
			}
		}
	}
	return 0;
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
		for(int i = (int)mac->context.heap.size(); i > disto; i--) {
			backed.push_back(mac->context.heap.back());
			mac->context.heap.pop_back();
		}
	}
	mac->context.heap.push_back(&hipo);

	mac->context.isso.push_back(mac->context.imme);
	mac->context.imme = immcon.imme;

	for(unsigned int i = startpos+4; i < startpos+sz; i++) {
		uint32_t	msg = mac->context.code[i] & tsbcmsg;
		uint32_t	bco = mac->context.code[i] &~tsbcmsg;
		msg >>= 32UL-4UL;
		if(msg == ts_stop) {
			if(bco == 0) bco = 1;
			mac->execline(i);
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

