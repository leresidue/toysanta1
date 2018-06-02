#pragma once


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


class TEXTtoy : public toyFUNC {
public:
	TEXTtoy();

	tsbcode do_just(toyCONTEXT *ctx, int it, size_t csz);

	int func(toyCONTEXT *ctx, int it, size_t csz);
	bool okconvert(tsbcode cd);
	std::wstring convert(toyCONTEXT *ctx, tsbcode code, int it);
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

class COUNTtoy : public funcGAME {
public:
	COUNTtoy();
	void withNUM(cgame *game);
};

class COMPAREtoy : public funcGAME {
public:
	enum {
		c_equals,

	} action = c_equals;
	void compare(cgame *game, std::wstring *s1, std::wstring *s2);
	void compare(cgame *game, int64_t s1, int64_t s2);
	void compare(cgame *game, double s1, double s2);
	COMPAREtoy(std::wstring str=L"==");
	void withNUM(cgame *game);
	void withSTR(cgame *game);
};
