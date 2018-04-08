//bind callable
#define bc(f) add(expr::make_smart_callable(f),#f)

//bind callable with name
#define bcn(f,n) add(expr::make_smart_callable(f),n)

//bind set
#define bs(s) use(expr::get_all_functions<s>(),#s)

//bind set with name
#define bsn(s,n) use(expr::get_all_functions<s>(),n)

//bind variable
#define bv(v) add(expr::make_smart_callable(std::function<decltype(v)*()>{[val=&v](){return val;}}),#v)

//bind variable with name
#define bvn(v,n) add(expr::make_smart_callable(std::function<decltype(v)*()>{[val=&v](){return val;}}),n)