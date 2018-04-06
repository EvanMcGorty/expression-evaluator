
#define asfn(f,n) add(expr::make_smart_callable(f),n)

#define asf(f) asfn(f,#f)
#define ufs(s) use(s::all(),#s)

