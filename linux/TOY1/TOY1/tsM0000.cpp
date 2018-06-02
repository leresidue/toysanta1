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
				} else if(ret.size()==0 && (src[i] == L'@' || src[i] == L'#')) {
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
							if( fnd->okc == tbc /*&& fnd->othp == thp*/) {
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
							//fnd->othp = ti->parent;
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
		//exec(exec_pos);
		//exec_pos = mac->context.code.size();
		//err = exec_pos;
		err = mac->context.code.size();
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
		if(toy[i] == L'|' || toy[i] == L'\n' || toy[i] == L'\r') {
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