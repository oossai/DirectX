#ifndef THROW_IF_FAILED
#define THROW_IF_FAILED(x)\
{\
	HRESULT hr = (x);\
	if (FAILED(hr))\
	{\
		throw Error{L#x, TEXT(__FILE__), __LINE__, hr};\
	}\
}
#endif

#ifndef NO_COPY_OR_MOVE
#define NO_COPY_OR_MOVE(className) \
    private: \
        className(const className&) = delete; \
        className(className&&) = delete; \
        className& operator=(const className&) = delete; \
        className& operator=(className&&) = delete;
#endif

#ifndef ASSIMP_PREPROCESS_FLAGS
#define ASSIMP_PREPROCESS_FLAGS (aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded)
#endif