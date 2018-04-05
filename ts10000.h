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


#pragma once
#define	tsbcmsg	0xf0000000UL
enum tsbtype {
	ts_num,	// 0
	ts_dec,	// 1
	ts_txt,	// 2
	ts_pnum,// 3
	ts_pdec,// 4
	ts_ptxt,// 5

	ts_anum,// 6
	ts_adec,// 7
	ts_atxt,// 8

	ts_func,// 9
	ts_code,// 10
	ts_stop,// 11
	ts_ends,// 12

	ts_cash,// 13
	ts_private,
	ts_protect = 15
};
typedef uint32_t tsbcode;
#define invalidtsb	0xffffffffUL
#define mktsbc(x, y) (tsbcode)((((uint32_t)x)|(tsbcmsg&(((uint32_t)y)<<(32UL-4UL)))))

extern class toyMACHINE;
extern class toyCONTEXT;
extern class toyCODE;


class toyFUNC {
public:
	std::wstring	name;
	bool can_var = false;
	bool is_code = false;
	bool target_me = false;
	bool just_auto = false;
	bool auto_exec = false;
	virtual tsbcode do_just(toyCONTEXT *ctx, int it, size_t csz);
	virtual int func(toyCONTEXT *ctx, int it, size_t csz);
	virtual bool okconvert(tsbcode cd);
	virtual std::wstring convert(toyCONTEXT *ctx, tsbcode code, int it);
	
};



template <typename T>
class toyVARS {
public:
	std::vector<T>				vars;
	std::vector<std::wstring>	names;

	int getpos(std::wstring src, int offset);
};

class toyHEAP {
public:
	toyVARS<int64_t>		num;
	toyVARS<double>			dec;
	toyVARS<std::wstring>	txt;

	toyVARS<int64_t>		pnum;
	toyVARS<double>			pdec;
	toyVARS<std::wstring>	ptxt;

	std::vector<int64_t>		anon_num;
	std::vector<double>			anon_dec;
	std::vector<std::wstring>	anon_str;

	uint32_t	offsets[9];

	std::vector<toyFUNC*>	func;
	std::vector<toyCODE*>	subs;

	uint32_t	func_offset;
	uint32_t	subs_offset;

	int64_t	dollar;

	toyHEAP();
	~toyHEAP();
	tsbcode getcorrect(std::wstring src);
	tsbcode getasgnpos(std::wstring src, bool known_p = true);
};
class toyPAIR;
class toyIMMEDIAT {
public:
	toyIMMEDIAT	*prev = nullptr;
	toyHEAP		*parent; int distance;
	uint32_t	offsets[9];
	uint32_t	func_offset;
	uint32_t	subs_offset;
	tsbcode		backfunc = invalidtsb;
	toyIMMEDIAT();
	void branchthrough(toyCONTEXT *ctx);
	int	code_position;
	toyMACHINE	*mac = nullptr;

	toyPAIR		*imme;
	/*
		
	*/
};

class toyCODE {
public:
	toyIMMEDIAT	immcon;

	tsbcode		prudence = ts_protect;

	toyHEAP		sheap;
	uint32_t	startpos, sz;
	bool is_code = true;
	bool cant_just = false;
	std::wstring	name;
	toyCODE();
	virtual int code(toyMACHINE *mac, int it, size_t csz);
};

class toyPAIR {
public:
	toyPAIR	*next = nullptr;
	tsbcode	okc = invalidtsb;
	int			code_position;
	toyIMMEDIAT	*ti = nullptr;

};

class toyCONTEXT {
protected:
	bool test_ok_exec(tsbcode okc, toyHEAP *opth, int it);
public:
	toyHEAP				baseheap;
	//std::vector<toyFUNC*>	func;
	std::vector<toyHEAP*>	heap;
	std::vector<tsbcode>	code;
	std::vector<size_t>		cstack;
	//std::vector<toyPAIR*>	imme;
	//toyPAIR		*imme = nullptr;
	std::vector<toyPAIR*>	isso;
	toyPAIR					*imme = nullptr;
	//std::vector<toyCODE*>	subs;

	//uint32_t	func_offset;
	//uint32_t	subs_offset;
	size_t	call_position = 0;
	size_t	code_position = 0;

	toyCONTEXT();
	~toyCONTEXT();
	void prepare();

	int64_t *getdollar();
	tsbcode startresolve(std::wstring vnam);
	tsbcode resolvevar(std::wstring vnam);

	double *finddec(tsbcode cd, tsbtype tp = ts_ends);
	int64_t *findnum(tsbcode cd, tsbtype tp = ts_ends);
	std::wstring *findstr(tsbcode cd, int it, tsbtype tp = ts_ends, bool oktest = true, toyHEAP **opth = nullptr, tsbcode *okc = nullptr);

	toyFUNC *findtoy(tsbcode cd);
	toyCODE *findsub(tsbcode cd);
	tsbcode getsubpos(std::wstring src);
	tsbcode getfuncpos(std::wstring src);
	tsbcode getasgnpos(std::wstring src, bool known_p);

	tsbcode allocnum(int64_t src, std::wstring vnam, tsbtype msg);

	tsbcode allocdec(double src, std::wstring vnam, tsbtype msg);

	tsbcode alloctext(std::wstring src, std::wstring vnam, tsbtype msg);

	tsbcode allocanondec(double dec);
	tsbcode allocanonnum(int64_t num);
	tsbcode allocanonstr(std::wstring src);

	tsbcode alloccode(toyCODE *mc);

	void codepush(tsbcode cd);

};

class cgame {
protected:
	size_t		scsz;
public:
	toyCONTEXT	*ctx;
	int			it;
	size_t		csz;
	std::wstring	*cis = nullptr;
	double			*cid = nullptr;
	int64_t			*cip = nullptr;
	cgame(toyCONTEXT *pctx, int pti, size_t pcsz);
	tsbtype identify();
	bool domore();
	std::wstring *getstring();
};

class funcGAME : public toyFUNC {
public:
	funcGAME(const wchar_t *pna = L"");
	int func(toyCONTEXT *ctx, int it, size_t csz);
	virtual void withSTR(cgame *game);
	virtual void withDEC(cgame *game);
	virtual void withNUM(cgame *game);
};

class COUNTtoy : public funcGAME {
public:
	COUNTtoy();
	void withNUM(cgame *game);
};

class PRINTtoy : public toyFUNC {
public:
	PRINTtoy();
	int func(toyCONTEXT *ctx, int it, size_t csz);
};

class INPUTtoy : public toyFUNC {
public:
	INPUTtoy();
	int func(toyCONTEXT *ctx, int it, size_t csz);
};

class CONVERTtoy : public toyFUNC {
public:
	CONVERTtoy();
	int func(toyCONTEXT *ctx, int it, size_t csz);
};

class FINDtoy : public toyFUNC {
public:
	enum {
		f_find,
		f_count,
	} action = f_find;
	FINDtoy(const wchar_t *pna = L"find");
	int func(toyCONTEXT *ctx, int it, size_t csz);
};

class ASSIGNtoy : public toyFUNC {
public:
	enum {
		a_assign,
		a_equals,
		a_lesser,
		a_more,

		a_expose,
		a_square,

		a_modulo,

		a_minus,
		a_plus,

		a_mul,
		a_div,

		a_tickcount,
	} action = a_assign;
	bool	as_accu = false;
	bool	as_init = false;
	ASSIGNtoy(const wchar_t *pna = L"=");
	int do_d64(int64_t *d64, int64_t elem);
	int do_f64(double *d64, double elem);
	int do_dstr(std::wstring *d64, const std::wstring &elem);
	int func(toyCONTEXT *ctx, int it, size_t csz);
	
};

class TEXTtoy : public toyFUNC {
public:
	TEXTtoy();

	tsbcode do_just(toyCONTEXT *ctx, int it, size_t csz);

	int func(toyCONTEXT *ctx, int it, size_t csz);
	bool okconvert(tsbcode cd);
	std::wstring convert(toyCONTEXT *ctx, tsbcode code, int it);
};

class INTEGERtoy : public toyFUNC {
public:
	enum {
		a_integer,
		a_decimal,
	} action = a_integer;
	INTEGERtoy(const wchar_t *pn = L"integer");
	int func(toyCONTEXT *ctx, int it, size_t csz);
	tsbcode do_just(toyCONTEXT *ctx, int it, size_t csz);
	int dfunc(toyCONTEXT *ctx, const tsbcode *code, size_t csz, int it);
	bool okconvert(tsbcode cd);
	
};

class MACROtoy : public toyFUNC {
public:
	MACROtoy();
	int func(toyCONTEXT *ctx, int it, size_t csz);
	tsbcode do_just(toyCONTEXT *ctx, int it, size_t csz);
};

class IFcode : public toyCODE {
public:
	IFcode();
	int code(toyMACHINE *mac, int it, size_t csz);
};

class LOOPcode : public toyCODE {
public:
	LOOPcode();
	int code(toyMACHINE *mac, int it, size_t csz);
};

class MACROcode : public toyCODE {
public:
	MACROcode();
	int code(toyMACHINE *mac, int it, size_t csz);
};
class toyMAKER;
class toyMACHINE {
protected:
	friend class toyMAKER;
	toyFUNC	*metarget = nullptr;

	bool ok_wait_exec(toyFUNC *fn, int cpo);
public:
	toyCONTEXT	context;

	tsbcode			MACHINEignore;
	std::vector<toyCODE*>	bank;

	toyMACHINE();
	~toyMACHINE();
	void prepare();

	int64_t *getdollar();
	void do_txt_convert(std::wstring src);

	int do_asgn_parm(const wchar_t *src, int sz, toyCODE *cT, bool is_code);

	int do_text_parm(const wchar_t *src, int sz, toyCODE *cT);

	bool acceptfuncchar(std::wstring src, int pos, int sz);
	bool acceptasgnchar(std::wstring src, int pos, int sz);

	std::wstring getfunc(std::wstring src, int *pos);
	std::wstring getasgn(std::wstring src, int *pos);
	std::wstring getparm(std::wstring src, int *pos);

	int feedline(std::wstring src, std::wstring stop2, std::wstring *stop3);
	int execsolo(int it, size_t csz);
	int execcode(int it, size_t csz);
	int execline(int err);
};

class toyMAKER {
public:
	std::wstring	targetstr;
	int				exec_pos = 0;
	toyMACHINE	*target = nullptr;
	std::vector<int>	code_subs, heaps;
	std::vector<std::wstring>	code_labels;
	int make2(std::wstring toy, toyMACHINE *mac);
	int make(std::wstring toy, toyMACHINE *mac);

	void exec(int perr);
};

