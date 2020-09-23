#ifndef MROCCLUSION_NONCOPYABLE_HPP_INCLUDED
#define MROCCLUSION_NONCOPYABLE_HPP_INCLUDED

class NonCopyable
{
public:
	NonCopyable() {}
private:
	NonCopyable(const NonCopyable &other);
	NonCopyable &operator=(const NonCopyable &other);
};

#endif
