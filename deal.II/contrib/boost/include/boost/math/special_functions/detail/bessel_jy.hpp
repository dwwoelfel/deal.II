//  Copyright (c) 2006 Xiaogang Zhang
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_BESSEL_JY_HPP
#define BOOST_MATH_BESSEL_JY_HPP

#ifdef _MSC_VER
#pragma once
#endif

#include <boost/math/tools/config.hpp>
#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/sign.hpp>
#include <boost/math/special_functions/hypot.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/detail/simple_complex.hpp>
#include <boost/math/special_functions/detail/bessel_jy_asym.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/math/policies/error_handling.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <complex>

// Bessel functions of the first and second kind of fractional order

namespace boost { namespace math {

namespace detail {

// Calculate Y(v, x) and Y(v+1, x) by Temme's method, see
// Temme, Journal of Computational Physics, vol 21, 343 (1976)
template <typename T, typename Policy>
int temme_jy(T v, T x, T* Y, T* Y1, const Policy& pol)
{
    T g, h, p, q, f, coef, sum, sum1, tolerance;
    T a, d, e, sigma;
    unsigned long k;

    BOOST_MATH_STD_USING
    using namespace boost::math::tools;
    using namespace boost::math::constants;

    BOOST_ASSERT(fabs(v) <= 0.5f);  // precondition for using this routine

    T gp = boost::math::tgamma1pm1(v, pol);
    T gm = boost::math::tgamma1pm1(-v, pol);
    T spv = boost::math::sin_pi(v, pol);
    T spv2 = boost::math::sin_pi(v/2, pol);
    T xp = pow(x/2, v);

    a = log(x / 2);
    sigma = -a * v;
    d = abs(sigma) < tools::epsilon<T>() ?
        T(1) : sinh(sigma) / sigma;
    e = abs(v) < tools::epsilon<T>() ? v*pi<T>()*pi<T>() / 2
        : 2 * spv2 * spv2 / v;

    T g1 = (v == 0) ? -euler<T>() : (gp - gm) / ((1 + gp) * (1 + gm) * 2 * v);
    T g2 = (2 + gp + gm) / ((1 + gp) * (1 + gm) * 2);
    T vspv = (fabs(v) < tools::epsilon<T>()) ? 1/constants::pi<T>() : v / spv;
    f = (g1 * cosh(sigma) - g2 * a * d) * 2 * vspv;

    p = vspv / (xp * (1 + gm));
    q = vspv * xp / (1 + gp);

    g = f + e * q;
    h = p;
    coef = 1;
    sum = coef * g;
    sum1 = coef * h;

    T v2 = v * v;
    T coef_mult = -x * x / 4;

    // series summation
    tolerance = tools::epsilon<T>();
    for (k = 1; k < policies::get_max_series_iterations<Policy>(); k++)
    {
        f = (k * f + p + q) / (k*k - v2);
        p /= k - v;
        q /= k + v;
        g = f + e * q;
        h = p - k * g;
        coef *= coef_mult / k;
        sum += coef * g;
        sum1 += coef * h;
        if (abs(coef * g) < abs(sum) * tolerance) 
        { 
           break; 
        }
    }
    policies::check_series_iterations("boost::math::bessel_jy<%1%>(%1%,%1%) in temme_jy", k, pol);
    *Y = -sum;
    *Y1 = -2 * sum1 / x;

    return 0;
}

// Evaluate continued fraction fv = J_(v+1) / J_v, see
// Abramowitz and Stegun, Handbook of Mathematical Functions, 1972, 9.1.73
template <typename T, typename Policy>
int CF1_jy(T v, T x, T* fv, int* sign, const Policy& pol)
{
    T C, D, f, a, b, delta, tiny, tolerance;
    unsigned long k;
    int s = 1;

    BOOST_MATH_STD_USING

    // |x| <= |v|, CF1_jy converges rapidly
    // |x| > |v|, CF1_jy needs O(|x|) iterations to converge

    // modified Lentz's method, see
    // Lentz, Applied Optics, vol 15, 668 (1976)
    tolerance = 2 * tools::epsilon<T>();
    tiny = sqrt(tools::min_value<T>());
    C = f = tiny;                           // b0 = 0, replace with tiny
    D = 0.0L;
    for (k = 1; k < policies::get_max_series_iterations<Policy>() * 100; k++)
    {
        a = -1;
        b = 2 * (v + k) / x;
        C = b + a / C;
        D = b + a * D;
        if (C == 0) { C = tiny; }
        if (D == 0) { D = tiny; }
        D = 1 / D;
        delta = C * D;
        f *= delta;
        if (D < 0) { s = -s; }
        if (abs(delta - 1.0L) < tolerance) 
        { break; }
    }
    policies::check_series_iterations("boost::math::bessel_jy<%1%>(%1%,%1%) in CF1_jy", k / 100, pol);
    *fv = -f;
    *sign = s;                              // sign of denominator

    return 0;
}

template <class T>
struct complex_trait
{
   typedef typename mpl::if_<is_floating_point<T>,
      std::complex<T>, sc::simple_complex<T> >::type type;
};

// Evaluate continued fraction p + iq = (J' + iY') / (J + iY), see
// Press et al, Numerical Recipes in C, 2nd edition, 1992
template <typename T, typename Policy>
int CF2_jy(T v, T x, T* p, T* q, const Policy& pol)
{
    BOOST_MATH_STD_USING

    typedef typename complex_trait<T>::type complex_type;

    complex_type C, D, f, a, b, delta, one(1);
    T tiny, zero(0.0L);
    unsigned long k;

    // |x| >= |v|, CF2_jy converges rapidly
    // |x| -> 0, CF2_jy fails to converge
    BOOST_ASSERT(fabs(x) > 1);

    // modified Lentz's method, complex numbers involved, see
    // Lentz, Applied Optics, vol 15, 668 (1976)
    T tolerance = 2 * tools::epsilon<T>();
    tiny = sqrt(tools::min_value<T>());
    C = f = complex_type(-0.5f/x, 1.0L);
    D = 0;
    for (k = 1; k < policies::get_max_series_iterations<Policy>(); k++)
    {
        a = (k - 0.5f)*(k - 0.5f) - v*v;
        if (k == 1)
        {
            a *= complex_type(T(0), 1/x);
        }
        b = complex_type(2*x, T(2*k));
        C = b + a / C;
        D = b + a * D;
        if (C == zero) { C = tiny; }
        if (D == zero) { D = tiny; }
        D = one / D;
        delta = C * D;
        f *= delta;
        if (abs(delta - one) < tolerance) { break; }
    }
    policies::check_series_iterations("boost::math::bessel_jy<%1%>(%1%,%1%) in CF2_jy", k, pol);
    *p = real(f);
    *q = imag(f);

    return 0;
}

enum
{
   need_j = 1, need_y = 2
};

// Compute J(v, x) and Y(v, x) simultaneously by Steed's method, see
// Barnett et al, Computer Physics Communications, vol 8, 377 (1974)
template <typename T, typename Policy>
int bessel_jy(T v, T x, T* J, T* Y, int kind, const Policy& pol)
{
    BOOST_ASSERT(x >= 0);

    T u, Jv, Ju, Yv, Yv1, Yu, Yu1(0), fv, fu;
    T W, p, q, gamma, current, prev, next;
    bool reflect = false;
    unsigned n, k;
    int s;

    static const char* function = "boost::math::bessel_jy<%1%>(%1%,%1%)";

    BOOST_MATH_STD_USING
    using namespace boost::math::tools;
    using namespace boost::math::constants;

    if (v < 0)
    {
        reflect = true;
        v = -v;                             // v is non-negative from here
        kind = need_j|need_y;               // need both for reflection formula
    }
    n = iround(v, pol);
    u = v - n;                              // -1/2 <= u < 1/2

    if (x == 0)
    {
       *J = *Y = policies::raise_overflow_error<T>(
          function, 0, pol);
       return 1;
    }

    // x is positive until reflection
    W = T(2) / (x * pi<T>());               // Wronskian
    if (x <= 2)                           // x in (0, 2]
    {
        if(temme_jy(u, x, &Yu, &Yu1, pol))             // Temme series
        {
           // domain error:
           *J = *Y = Yu;
           return 1;
        }
        prev = Yu;
        current = Yu1;
        for (k = 1; k <= n; k++)            // forward recurrence for Y
        {
            next = 2 * (u + k) * current / x - prev;
            prev = current;
            current = next;
        }
        Yv = prev;
        Yv1 = current;
        if(kind&need_j)
        {
          CF1_jy(v, x, &fv, &s, pol);                 // continued fraction CF1_jy
          Jv = W / (Yv * fv - Yv1);           // Wronskian relation
        }
        else
           Jv = std::numeric_limits<T>::quiet_NaN(); // any value will do, we're not using it.
    }
    else                                    // x in (2, \infty)
    {
        // Get Y(u, x):
        // define tag type that will dispatch to right limits:
        typedef typename bessel_asymptotic_tag<T, Policy>::type tag_type;

        T lim;
        switch(kind)
        {
        case need_j:
           lim = asymptotic_bessel_j_limit<T>(v, tag_type());
           break;
        case need_y:
           lim = asymptotic_bessel_y_limit<T>(tag_type());
           break;
        default:
           lim = (std::max)(
              asymptotic_bessel_j_limit<T>(v, tag_type()),
              asymptotic_bessel_y_limit<T>(tag_type()));
           break;
        }
        if(x > lim)
        {
           if(kind&need_y)
           {
              Yu = asymptotic_bessel_y_large_x_2(u, x);
              Yu1 = asymptotic_bessel_y_large_x_2(u + 1, x);
           }
           else
              Yu = std::numeric_limits<T>::quiet_NaN(); // any value will do, we're not using it.
           if(kind&need_j)
           {
              Jv = asymptotic_bessel_j_large_x_2(v, x);
           }
           else
              Jv = std::numeric_limits<T>::quiet_NaN(); // any value will do, we're not using it.
        }
        else
        {
           CF1_jy(v, x, &fv, &s, pol);
           // tiny initial value to prevent overflow
           T init = sqrt(tools::min_value<T>());
           prev = fv * s * init;
           current = s * init;
           for (k = n; k > 0; k--)             // backward recurrence for J
           {
               next = 2 * (u + k) * current / x - prev;
               prev = current;
               current = next;
           }
           T ratio = (s * init) / current;     // scaling ratio
           // can also call CF1_jy() to get fu, not much difference in precision
           fu = prev / current;
           CF2_jy(u, x, &p, &q, pol);                  // continued fraction CF2_jy
           T t = u / x - fu;                   // t = J'/J
           gamma = (p - t) / q;
           Ju = sign(current) * sqrt(W / (q + gamma * (p - t)));

           Jv = Ju * ratio;                    // normalization

           Yu = gamma * Ju;
           Yu1 = Yu * (u/x - p - q/gamma);
        }
        if(kind&need_y)
        {
           // compute Y:
           prev = Yu;
           current = Yu1;
           for (k = 1; k <= n; k++)            // forward recurrence for Y
           {
               next = 2 * (u + k) * current / x - prev;
               prev = current;
               current = next;
           }
           Yv = prev;
        }
        else
           Yv = std::numeric_limits<T>::quiet_NaN(); // any value will do, we're not using it.
    }

    if (reflect)
    {
        T z = (u + n % 2);
        *J = boost::math::cos_pi(z, pol) * Jv - boost::math::sin_pi(z, pol) * Yv;     // reflection formula
        *Y = boost::math::sin_pi(z, pol) * Jv + boost::math::cos_pi(z, pol) * Yv;
    }
    else
    {
        *J = Jv;
        *Y = Yv;
    }

    return 0;
}

} // namespace detail

}} // namespaces

#endif // BOOST_MATH_BESSEL_JY_HPP

