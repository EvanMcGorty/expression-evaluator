#define bc(f) add(expr::make_smart_callable(f),#f)
#define bcn(f,n) add(expr::make_smart_callable(f),n)
#define bs(s) use(s::all(),#s)
#define bsn(s,n) use(s::all(),n)