int neg_const() { return -5; }

int neg_var(int x) { return -x; }

int lnot_zero() { return !0; }

int lnot_nonzero() { return !42; }

int lnot_var(int x) { return !x; }

int double_lnot(int x) { return !!x; }
