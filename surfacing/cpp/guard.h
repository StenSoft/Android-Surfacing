#pragma once

#include <utility>


/**
 * Guard.
 *
 * Takes a lambda and calls it in destructor. Use to implement RAII on-the-fly.
 */
template <typename LambdaT>
class Guard
{
public:
    Guard(LambdaT && lambda)
        : mLambda(std::forward<LambdaT>(lambda))
    {}

    ~Guard()
    {
        mLambda();
    }

private:
    Guard(Guard const&) = delete;
    Guard(Guard &&) = delete;
    Guard& operator =(Guard const&) = delete;
    Guard& operator =(Guard &&) = delete;

    LambdaT mLambda;
};


/**
 * Create a guard.
 *
 * @param lambda Lambda to guard with.
 */
template <typename LambdaT>
auto guard(LambdaT && lambda)
{
    return Guard<LambdaT>(std::forward<LambdaT>(lambda));
}
