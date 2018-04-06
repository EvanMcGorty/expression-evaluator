
#define ufs(s) use(s::all(),#s)

#define FC_ARBITRARY_ARGSO_ARGS() std::static_assert(false,"can't call fc without arguments")
#define FC_ONE_ARGS(A) add(expr::make_smart_callable(A),#A)
#define FC_TWO_ARGS(A,B) add(expr::make_smart_callable(A),B)

#define FC_ARBITRARY_ARGS(x,A,B,FUNC, ...) FUNC

#define fc(...) FC_ARBITRARY_ARGS(,##__VA_ARGS__, FC_TWO_ARGS(__VA_ARGS__), FC_ONE_ARGS(__VA_ARGS__), FC_ARBITRARY_ARGSO_ARGS(__VA_ARGS__))

#define FS_ARBITRARY_ARGSO_ARGS() std::static_assert(false,"can't call fs without arguments")
#define FS_ONE_ARGS(A) use(A::all(),#A)
#define FS_TWO_ARGS(A,B) use(A::all(),#A)

#define FS_ARBITRARY_ARGS(x,A,B,FUNC, ...) FUNC

#define fs(...) FS_ARBITRARY_ARGS(,##__VA_ARGS__, FS_TWO_ARGS(__VA_ARGS__), FS_ONE_ARGS(__VA_ARGS__), FS_ARBITRARY_ARGSO_ARGS(__VA_ARGS__))

