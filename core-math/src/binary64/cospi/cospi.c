/* Correctly-rounded cosine of binary64 value for angles in half-revolutions

Copyright (c) 2023-2026 Alexei Sibidanov <sibid@uvic.ca>.

This file is part of the CORE-MATH project
(https://core-math.gitlabpages.inria.fr/).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <stdint.h>
#ifdef CORE_MATH_SUPPORT_ERRNO
#include <errno.h>
#endif
#include <fenv.h> // for feraiseexcept, FE_INVALID
#include <math.h> // needed to provide cospi() since glibc does not have it

// Warning: clang also defines __GNUC__
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#pragma STDC FENV_ACCESS ON

typedef union {double f; uint64_t u;} b64u64_u;

static void sincosn(int, double*, double*, double*, double*);
static void sincosn2(int, double*, double*, double*, double*);

static inline double fasttwosum(double x, double y, double *e){
  double s = x + y, z = s - x;
  *e = y - z;
  return s;
}

static inline double muldd_acc(double xh, double xl, double ch, double cl, double *l){
  double ahlh = ch*xl, alhh = cl*xh, ahhh = ch*xh, ahhl = __builtin_fma(ch, xh, -ahhh);
  ahhl += alhh + ahlh;
  return fasttwosum (ahhh, ahhl, l);
}

static inline double mulddd(double xh, double xl, double ch, double *l){
  double ahlh = ch*xl, ahhh = ch*xh, ahhl = __builtin_fma(ch, xh, -ahhh);
  ahhl += ahlh;
  ch = ahhh + ahhl;
  *l = (ahhh - ch) + ahhl;
  return ch;
}

static inline double polydd(double xh, double xl, int n, const double c[][2], double *l){
  int i = n-1;
  double ch = c[i][0] + *l, cl = ((c[i][0] - ch) + *l) + c[i][1];
  while(--i>=0){
    ch = muldd_acc(xh, xl, ch, cl, &cl);
    double th = ch + c[i][0], tl = (c[i][0] - th) + ch;
    ch = th;
    cl += tl + c[i][1];
  }
  *l = cl;
  return ch;
}

static double as_cospi_zero(double x){
  double x2 = x*x, dx2 = __builtin_fma(x,x,-x2);
  static const double ch[][2] = {
    {-0x1.3bd3cc9be45dep+2, -0x1.692b71366cc04p-52}, {0x1.03c1f081b5ac4p+2, -0x1.32b33fda9113cp-52}};
  static const double cl[3] = {-0x1.55d3c7e3cbff9p+0, 0x1.e1f50604fa0ffp-3};
  double fl = x2*(cl[0] + x2*(cl[1]));
  double fh = polydd(x2, dx2, 2, ch, &fl);
  fh = muldd_acc(x2, dx2, fh, fl, &fl);
  double y2, y1, y0 = fasttwosum(1, fh, &y1);
  y1 = fasttwosum(y1, fl, &y2);
  b64u64_u t = {.f = y1};
  if(__builtin_expect(!(t.u&(~0ull>>12)), 0)){
    b64u64_u w = {.f = y2};
    if((w.u^t.u)>>63)
      t.u--;
    else
      t.u++;
    y1 = t.f;
  }
  return y0 + y1;
}

static double as_cospi_refine(int iq, double z){
  double x = z*0x1p-63, x2 = x*x, dx2 = __builtin_fma(x,x,-x2);
  static const double sh[][2] = {
    {0x1.921fb54442d18p+1, 0x1.1a62633145c06p-53}, {-0x1.4abbce625be53p-22, 0x1.05511cbc65743p-76},
    {0x1.466bc6775aae1p-47, -0x1.9c3c168d990ap-114}};
  static const double ch[][2] = {
    {-0x1.3bd3cc9be45dep-22, -0x1.692b71366cc04p-76}, {0x1.03c1f081b5ac4p-46, -0x1.32b33fda9113cp-100}};
  double sll = -0x1.32d2cc920dcb4p-73*x2;
  double slh = polydd(x2, dx2, 3, sh, &sll);
  slh = mulddd(slh, sll, x*0x1p-12, &sll);
  double cll = x2*(-0x1.55d3c7e3cbff9p-72 + 0x1.e1f50604fa0ffp-99*x2);
  double clh = polydd(x2, dx2, 2, ch, &cll);
  clh = muldd_acc(clh, cll, x2, dx2, &cll);
  double sbh,sbl,cbh,cbl; sincosn2(iq,&sbh,&sbl,&cbh,&cbl);
  double csl, csh = muldd_acc(clh,cll, sbh,sbl, &csl);
  double scl, sch = muldd_acc(slh,sll, cbh,cbl, &scl);
  double tsl, tsh = fasttwosum(sch, csh, &tsl); tsl += csl + scl;
  double tsl2; tsh = fasttwosum(sbh, tsh, &tsl2); tsl = sbl + tsl + tsl2;
  b64u64_u t = {.f = tsl};
  if((t.u|((uint64_t)0xfff<<52)) == ~(uint64_t)0 || (t.u<<12) == 0){
    static const struct {int iq; double x, r, d;} db[] = {
      {903, -0x1.bdd02d1ad60p-2, 0x1.f72c906962631p-1,  0x1p-55},
      {1029, -0x1.a4ad070549dp-3, 0x1.fffc4d2c6ca51p-1,  0x1p-55},
      {1078, 0x1.fbdb79ca3dap-3, 0x1.fe3c8219054c3p-1, 0x1p-55},
      {1217, 0x1.c0ccee4ada2p-1, 0x1.e99fd53791bcfp-1, -0x1p-55},
      {1025, 0x1.b536647b1fe98p-2, 0x1.ffffc5ddd0738p-1, 0x1p-55},
      {1026,-0x1.dc93eaad12a18p-2, 0x1.ffff84b21c731p-1, 0x1p-55},
      {1033, 0x1.e78f0e592b36p-1, 0x1.fff2270422604p-1, -0x1p-55},
      {1235, 0x1.f13412a48d8p-2, 0x1.e55a7fa9a24c4p-1, -0x1p-55},
    };
    double sgn = iq>2048?-1:1;
    iq &= 0x7ff;
    for(unsigned i=0; i<sizeof(db)/sizeof(db[0]); i++)
      // if c1 is the 1st condition below and c2 the 2nd one, we cannot have c1=c2=1,
      if((x == db[i].x && iq == db[i].iq) || (x == -db[i].x && iq == 2048-db[i].iq))
        return sgn*db[i].r + sgn*db[i].d;
  }
  return tsh + tsl;
}

double cr_cospi(double x){
  static const double sn[] = { 0x1.921fb54442d18p-74, -0x1.4abbce625be51p-223, 0x1.466bc6044ba16p-374};
  static const double cn[] = {-0x1.3bd3cc9be45dbp-148, 0x1.03c1f00186416p-298};
  b64u64_u ix = {.f = x};
  uint64_t ax = ix.u&(~0ull>>1);
  if(__builtin_expect(ax==0, 0)) return 1.0;
  int32_t e = ax>>52;
  // e is the unbiased exponent, we have 2^(e-1023) <= |x| < 2^(e-1022)
  uint64_t m = (ix.u&(~0ull>>12))|((uint64_t)1<<52);
  int32_t s = 1063 - e; // 2^(40-s) <= |x| < 2^(41-s)
  if(__builtin_expect(s<0, 0)){ // |x| >= 2^41
    if(__builtin_expect(e == 0x7ff, 0)){ // NaN or Inf
      if(!(ix.u << 12)){
#ifdef CORE_MATH_SUPPORT_ERRNO
	errno = EDOM;
#endif
	feraiseexcept (FE_INVALID);
	return __builtin_nan("inf");
      }
      return x + x; // NaN
    }
    s = -s - 1; // now 2^(41+s) <= |x| < 2^(42+s)
    if(s>11) return 1.0; // |x| >= 2^53
    uint64_t iq = ((uint64_t)m<<s) + 1024;
    if(!(iq&2047)) return 0.0;
    double sh, sl, ch, cl; sincosn(iq, &sh, &sl, &ch, &cl);
    return sh + sl;
  }
  if(__builtin_expect(ax<=0x3f30000000000000ull, 0)){ // |x| <= 2^-12
    if(__builtin_expect(ax<=0x3e2ccf6429be6621ull, 0)) return 1.0 - 0x1p-55;
    double x2 = x*x, x4 = x2*x2, eps = x2*0x1.ap-48;
    static const double c[] = {-0x1.3bd3cc9be45dcp+2, 0x1.03c1f081b0833p+2, -0x1.55d3c6fc9af15p+0, 0x1.e1d3ff2ae3f9ap-3};
    double p = x2*((c[0] + x2*c[1]) + x4*(c[2] + x2*c[3]));
    double lb = (p - eps) + 1, ub = (p + eps) + 1;
    if(lb==ub) return lb;
    return as_cospi_zero(x);
  }
  
  int32_t si = e-1011;
  // we always have si >= 0 here
  if(__builtin_expect((m<<(si+1))==0, 0)) { // x is integer or half-integer
    if ((m<<si) == 0){ // x is integer
      int t = (m<<(si-1))>>63;
      // t = 0 if |x| = 1/2 mod 2, t = 1 if |x| = 3/2 mod 2
      return t ? -1.0 : 1.0;
    }
    return 0.0;
  }

  uint64_t iq = ((m>>s) + 2048)&8191;
  iq = (iq + 1)>>1;
  int64_t k = (uint64_t)m<<(e-1000);
  double z = k, z2 = z*z;
  double fs = sn[0] + z2*(sn[1] + z2*sn[2]);
  double fc = cn[0] + z2*cn[1];
  double sh,sl,ch,cl; sincosn(iq,&sh,&sl,&ch,&cl);
  double er = __builtin_fabs(z)*0x1p-123 + 0x1p-78;
  double r = sl + sh*(z2*fc) + ch*(z*fs);
  double lb = (r - er) + sh, ub = (r + er) + sh;
  if(__builtin_expect(lb == ub, 1)) return lb;
  return as_cospi_refine(iq, z);
}

void sincosn(int s, double *sh, double *sl, double *ch, double *cl){
  static const double Sn[][2] =
    {{0x0p+0, 0x0p+0}, {0x1.91f66p-5, -0x1.de44fd832257ap-30},
     {0x1.917a6cp-4, -0x1.eb25ea0f138c7p-31}, {0x1.2c8106p-3, 0x1.d1cc27444c003p-28},
     {0x1.8f8b84p-3, -0x1.cb2cfaa4da337p-30}, {0x1.f19f98p-3, -0x1.37a839542deefp-29},
     {0x1.294063p-2, -0x1.2a60fa574a369p-30}, {0x1.58f9a7p-2, 0x1.6ac7f73f8409p-28},
     {0x1.87de2ap-2, 0x1.abaa58b469891p-28}, {0x1.b5d101p-2, -0x1.87a8cff5264eap-28},
     {0x1.e2b5d4p-2, -0x1.fe4271387c9dcp-28}, {0x1.0738798p-1, 0x1.22ffed9697fafp-29},
     {0x1.1c73b38p-1, 0x1.ae68c86c9774ap-29}, {0x1.30ff8p-1, -0x1.8f47e58f7e631p-28},
     {0x1.44cf328p-1, -0x1.7b7114f3fc4afp-28}, {0x1.57d6938p-1, -0x1.b989b02eae413p-28},
     {0x1.6a09e68p-1, -0x1.80c4336f74d05p-29}, {0x1.7b5df2p-1, 0x1.3557d76f0ac85p-28},
     {0x1.8bc8068p-1, 0x1.8a8ba05a743dap-28}, {0x1.9b3e048p-1, -0x1.8f17e98771434p-34},
     {0x1.a9b6628p-1, 0x1.0ea1a3033ec62p-29}, {0x1.b728348p-1, -0x1.7348e1378d3e6p-28},
     {0x1.c38b2fp-1, 0x1.80bdb0d23e9d1p-29}, {0x1.ced7af8p-1, -0x1.e19c46879edafp-28},
     {0x1.d906bdp-1, -0x1.9ae573aea067cp-30}, {0x1.e212108p-1, -0x1.84bc8da0298eep-28},
     {0x1.e9f4158p-1, -0x1.39d225a27d387p-29}, {0x1.f0a7ef8p-1, 0x1.c9186b952c7aep-28},
     {0x1.f6297dp-1, -0x1.1469faa77a357p-34}, {0x1.fa7558p-1, -0x1.eeb5d2bd05465p-30},
     {0x1.fd88dap-1, 0x1.e89292cf04139p-28}, {0x1.ff621ep-1, 0x1.bcb6bef1d421fp-28}, {0x1p+0, 0x0p+0}};
  static const double Sm[][2] =
    {{0x0p+0, 0x0p+0}, {0x1.922p-10, -0x1.54466e349ee53p-28},
     {0x1.921f8p-9, 0x1.7d99497495d2p-30}, {0x1.2d978p-8, 0x1.17ccb5e27cb43p-31},
     {0x1.921fp-8, 0x1.fcce00e23572dp-29}, {0x1.f6a64p-8, 0x1.f9a2a3c5885cfp-28},
     {0x1.2d96cp-7, -0x1.e35ed1fa23d22p-28}, {0x1.5fdap-7, 0x1.bd602f04014c8p-30},
     {0x1.921d2p-7, -0x1.909c3dccf0e28p-34}, {0x1.c46p-7, -0x1.e1b7526ebf9f2p-31},
     {0x1.f6a2ap-7, -0x1.2a8cd06af94a2p-28}, {0x1.14727p-6, 0x1.b5ae26618769ep-31},
     {0x1.2d937p-6, -0x1.1073c40b25037p-28}, {0x1.46b44p-6, -0x1.f80c5f9200607p-28},
     {0x1.5fd4dp-6, 0x1.0fd5912ef3f57p-29}, {0x1.78f53p-6, 0x1.77727c10d8ca5p-28},
     {0x1.92156p-6, -0x1.0b933040d8eb2p-31}, {0x1.ab355p-6, -0x1.3abec0d92ae48p-28},
     {0x1.c454fp-6, 0x1.3394ec7229c28p-28}, {0x1.dd746p-6, -0x1.ce6d5319d653ep-28},
     {0x1.f6937p-6, 0x1.8e8e7807f600bp-29}, {0x1.07d92p-5, -0x1.9eea00d71246dp-35},
     {0x1.14686p-5, -0x1.25e9f40a5121ap-28}, {0x1.20f77p-5, 0x1.9d6238d09f231p-30},
     {0x1.2d8658p-5, -0x1.4d75465d2f213p-30}, {0x1.3a151p-5, -0x1.37bca0781d73ap-30},
     {0x1.46a398p-5, -0x1.0079e86eec954p-29}, {0x1.5331fp-5, -0x1.e222f8a75de87p-28},
     {0x1.5fc01p-5, -0x1.6b7995e4bb32dp-28}, {0x1.6c4df8p-5, -0x1.41523e139c56bp-32},
     {0x1.78dba8p-5, 0x1.2c3a342eb5f11p-28}, {0x1.85692p-5, 0x1.5da89e0b235cp-28}};
  static const double Cm[][2] =
    {{0x1p+0, 0x0p+0}, {0x1.ffffd88p-1, 0x1.61bb991af64f1p-31},
     {0x1.ffff62p-1, 0x1.621d01d2a6063p-29}, {0x1.fffe9c8p-1, 0x1.8f17465de1773p-28},
     {0x1.fffd888p-1, -0x1.38bab6d94c71dp-28}, {0x1.fffc25p-1, 0x1.6bb5dd7625bcdp-30},
     {0x1.fffa73p-1, -0x1.b439d8a4596p-28}, {0x1.fff8718p-1, 0x1.37ce2eec7251bp-29},
     {0x1.fff6218p-1, -0x1.646d24a88970ep-29}, {0x1.fff3828p-1, -0x1.9bb848bdb041ep-28},
     {0x1.fff094p-1, 0x1.e29de85718cc2p-28}, {0x1.ffed57p-1, 0x1.cc695b5e89e49p-28},
     {0x1.ffe9cb8p-1, -0x1.da572f6a4bccap-28}, {0x1.ffe5f08p-1, -0x1.0d43929b71f74p-28},
     {0x1.ffe1c68p-1, 0x1.c32ddd89aa147p-31}, {0x1.ffdd4d8p-1, 0x1.fbc7a9242ccf3p-28},
     {0x1.ffd886p-1, 0x1.099a19765595dp-30}, {0x1.ffd36f8p-1, -0x1.dbaff3c93cdc4p-29},
     {0x1.ffce0ap-1, -0x1.8eacc3af7ac55p-28}, {0x1.ffc8558p-1, -0x1.996f62d2df41dp-28},
     {0x1.ffc252p-1, -0x1.071603e8582dfp-28}, {0x1.ffbbff8p-1, 0x1.7e5454b8225e9p-31},
     {0x1.ffb55e8p-1, -0x1.ed0128d24b027p-28}, {0x1.ffae6ep-1, 0x1.5569984bd1a7ap-29},
     {0x1.ffa72fp-1, -0x1.08a362d33736dp-37}, {0x1.ff9fa1p-1, 0x1.a441ba9901fd5p-32},
     {0x1.ff97c4p-1, 0x1.04600a0a95596p-28}, {0x1.ff8f988p-1, -0x1.387d3a589fa3dp-28},
     {0x1.ff871d8p-1, 0x1.6dc0ef98b1c67p-28}, {0x1.ff7e54p-1, 0x1.01907c4c59658p-28},
     {0x1.ff753b8p-1, 0x1.8dc8b1e83ccffp-28}, {0x1.ff6bd48p-1, -0x1.c4bbb2c34804p-29}};
  
  int j = s&0x3ff, it = -((s>>10)&1);
  j = (~it&j) - ((uint32_t)it<<10) - (it&j);
  int is = j>>5, ic = 0x20 - is, jm = j&0x1f;
  int ss = (s>>11)&1;
  int sc = (((uint32_t)s+1024)>>11)&1;
  
  double sbh = Sn[is][0], sbl = Sn[is][1];
  double cbh = Sn[ic][0], cbl = Sn[ic][1];
  double slh = Sm[jm][0], sll = Sm[jm][1];
  double clh = Cm[jm][0], cll = Cm[jm][1];

  double sb = sbh + sbl, cb = cbh + cbl;
  double Ch = cbh*clh - sbh*slh, Cl = clh*cbl - slh*sbl + cb*cll - sb*sll;
  double Sh = sbh*clh + cbh*slh, Sl = slh*cbl + clh*sbl + cb*sll + sb*cll;

  double tch = Ch + Cl, tcl = (Ch - tch) + Cl;
  double tsh = Sh + Sl, tsl = (Sh - tsh) + Sl;

  static const double sgn[2] = {0.0, -0.0};
  *ch = __builtin_copysign(1.0, sgn[sc])*tch;
  *cl = __builtin_copysign(1.0, sgn[sc])*tcl;

  *sh = __builtin_copysign(1.0, sgn[ss])*tsh;
  *sl = __builtin_copysign(1.0, sgn[ss])*tsl;
}

void sincosn2(int s, double *sh, double *sl, double *ch, double *cl){
  static const double Sn[][2] =
    {{0x0p+0, 0x0p+0}, {0x1.91f65f10dd814p-5, -0x1.912bd0d569a9p-61},
     {0x1.917a6bc29b42cp-4, -0x1.e2718d26ed688p-60}, {0x1.2c8106e8e613ap-3, 0x1.13000a89a11ep-58},
     {0x1.8f8b83c69a60bp-3, -0x1.26d19b9ff8d82p-57}, {0x1.f19f97b215f1bp-3, -0x1.42deef11da2c4p-57},
     {0x1.294062ed59f06p-2, -0x1.5d28da2c4612dp-56}, {0x1.58f9a75ab1fddp-2, -0x1.efdc0d58cf62p-62},
     {0x1.87de2a6aea963p-2, -0x1.72cedd3d5a61p-57}, {0x1.b5d1009e15ccp-2, 0x1.5b362cb974183p-57},
     {0x1.e2b5d3806f63bp-2, 0x1.e0d891d3c6841p-58}, {0x1.073879922ffeep-1, -0x1.a5a014347406cp-55},
     {0x1.1c73b39ae68c8p-1, 0x1.b25dd267f66p-55}, {0x1.30ff7fce17035p-1, -0x1.efcc626f74a6fp-57},
     {0x1.44cf325091dd6p-1, 0x1.8076a2cfdc6b3p-57}, {0x1.57d69348cecap-1, -0x1.75720992bfbb2p-55},
     {0x1.6a09e667f3bcdp-1, -0x1.bdd3413b26456p-55}, {0x1.7b5df226aafafp-1, -0x1.0f537acdf0ad7p-56},
     {0x1.8bc806b151741p-1, -0x1.2c5e12ed1336dp-55}, {0x1.9b3e047f38741p-1, -0x1.30ee286712474p-55},
     {0x1.a9b66290ea1a3p-1, 0x1.9f630e8b6dac8p-60}, {0x1.b728345196e3ep-1, -0x1.bc69f324e6d61p-55},
     {0x1.c38b2f180bdb1p-1, -0x1.6e0b1757c8d07p-56}, {0x1.ced7af43cc773p-1, -0x1.e7b6bb5ab58aep-58},
     {0x1.d906bcf328d46p-1, 0x1.457e610231ac2p-56}, {0x1.e212104f686e5p-1, -0x1.014c76c126527p-55},
     {0x1.e9f4156c62ddap-1, 0x1.760b1e2e3f81ep-55}, {0x1.f0a7efb9230d7p-1, 0x1.52c7adc6b4989p-56},
     {0x1.f6297cff75cbp-1, 0x1.562172a361fd3p-56}, {0x1.fa7557f08a517p-1, -0x1.7a0a8ca13571fp-55},
     {0x1.fd88da3d12526p-1, -0x1.87df6378811c7p-55}, {0x1.ff621e3796d7ep-1, -0x1.c57bc2e24aa15p-57}, {0x1p+0, 0x0p+0}};
  static const double Sm[][2] =
    {{0x0p+0, 0x0p+0}, {0x1.921faaee6472ep-10, -0x1.ee52e284a9df8p-64},
     {0x1.921f8becca4bap-9, 0x1.2ba407bcab5b2p-63}, {0x1.2d97822f996bcp-8, 0x1.3e5a15ed6aa3ep-62},
     {0x1.921f0fe670071p-8, 0x1.ab967fe6b7a9bp-64}, {0x1.f6a65f9a2a3c6p-8, -0x1.de8c48783f3aep-62},
     {0x1.2d96b0e509703p-7, -0x1.1e9131ff52dc9p-63}, {0x1.5fda037ac05e1p-7, -0x1.ff59bf4b574eep-61},
     {0x1.921d1fcdec784p-7, 0x1.9878ebe836d9dp-61}, {0x1.c45ffe1e48ad9p-7, 0x1.4060e4bd32e79p-63},
     {0x1.f6a296ab997cbp-7, -0x1.f2943d8fe7033p-61}, {0x1.147270dad7133p-6, 0x1.8769e00e018p-63},
     {0x1.2d936bbe30efdp-6, 0x1.b5f91ee371d64p-61}, {0x1.46b4381fce81bp-6, 0x1.ff9f89fb65be3p-60},
     {0x1.5fd4d21fab226p-6, -0x1.0c0a91c37851cp-61}, {0x1.78f535ddc9f04p-6, 0x1.b194ad9b1aa97p-61},
     {0x1.92155f7a3667ep-6, -0x1.b1d63091a013p-64}, {0x1.ab354b1504fcap-6, -0x1.2ae47937cbdd3p-60},
     {0x1.c454f4ce53b1dp-6, -0x1.d63d7fef0e36cp-60}, {0x1.dd7458c64ab3ap-6, -0x1.d653df3fcc281p-60},
     {0x1.f693731d1cf01p-6, -0x1.3fe9bc66286c7p-66}, {0x1.07d91ff98458p-5, -0x1.ae248da7a9007p-60},
     {0x1.14685db42c17fp-5, -0x1.2890d277cb974p-59}, {0x1.20f770ceb11c7p-5, -0x1.ec1b9d46693b1p-59},
     {0x1.2d865759455cdp-5, 0x1.686f65ba93acp-61}, {0x1.3a150f6421afcp-5, 0x1.f8a318ba775fdp-60},
     {0x1.46a396ff86179p-5, 0x1.136ac00fa2da9p-61}, {0x1.5331ec3bba0ebp-5, 0x1.442f2a9dac128p-61},
     {0x1.5fc00d290cd43p-5, 0x1.a2669a693a8e1p-59}, {0x1.6c4df7d7d5b84p-5, -0x1.39c56a9bd0a9bp-60},
     {0x1.78dbaa5874686p-5, -0x1.4a0ef4035c29cp-60}, {0x1.856922bb513c1p-5, 0x1.91adfd607cb2bp-59}};
  static const double Cm[][2] =
    {{0x0p+0, 0x0p+0}, {0x1.3bd3c88cdca13p-20, 0x1.874628d2b6835p-74},
     {0x1.3bd3bc5fc5ab4p-18, -0x1.8a1bebf665cefp-75}, {0x1.634e1d173443dp-17, 0x1.198ff5804d8dcp-73},
     {0x1.3bd38bab6d94cp-16, 0x1.c73be2184804ep-70}, {0x1.ed7a51288a277p-16, -0x1.bcd47bf19555bp-70},
     {0x1.634da1cec522dp-15, -0x1.3fff35c1bdb61p-69}, {0x1.e39b20c7444e3p-15, 0x1.ae49f0be31e8cp-69},
     {0x1.3bd2c8da49511p-14, 0x1.70df810bcc0e2p-69}, {0x1.8fb66ee122f6cp-14, 0x1.079ff34bffa7ep-70},
     {0x1.ed7875885ea3ap-14, -0x1.983278baa11c2p-69}, {0x1.2a8c672d4942fp-13, -0x1.e4938f661aa14p-68},
     {0x1.634bb4ae5ed49p-13, 0x1.e64c904cc8156p-67}, {0x1.a0fa1a872536ep-13, 0x1.f7469ecc1331fp-68},
     {0x1.e3978f34889d9p-13, 0x1.5eb897cdc1b0dp-67}, {0x1.1592043856dbdp-12, 0x1.9865d9cdc3744p-67},
     {0x1.3bcfbd9979a27p-12, -0x1.595d548d9a586p-66}, {0x1.6484edd7f9e4ap-12, -0x1.91df31aaa6f7fp-68},
     {0x1.8fb18eacc3af8p-12, -0x1.4eaa508eec2b7p-66}, {0x1.bd55996f62d2ep-12, -0x1.7c51f7d8f9f71p-69},
     {0x1.ed71071603e86p-12, -0x1.f4827ccb50b62p-66}, {0x1.1001e81abab48p-11, -0x1.12f4a2e3616d5p-66},
     {0x1.2a86f68094692p-11, 0x1.604e1f8f76c27p-65}, {0x1.4647aaa599ed1p-11, -0x1.1a79a86fb8ff5p-65},
     {0x1.6344004228d8bp-11, 0x1.33736c96557c9p-65}, {0x1.817bf2ddf22b3p-11, 0x1.fc0555a81d729p-65},
     {0x1.a0ef7dcffafabp-11, 0x1.54d46b817bca4p-65}, {0x1.c19e9c3e9d2c5p-11, -0x1.70a980659e79p-70},
     {0x1.e389491f8833ap-11, 0x1.c7313beeab883p-65}, {0x1.0357bf9be0ecfp-10, -0x1.965827f33d907p-64},
     {0x1.15889c8dd385fp-10, 0x1.98094fab77b42p-67}, {0x1.2857389776587p-10, -0x1.bfdfce09aea7bp-64}};
  
  int j = s&0x3ff, it = -((s>>10)&1);
  j = (~it&j) - ((uint32_t)it<<10) - (it&j);
  int is = j>>5, ic = 0x20 - is, jm = j&0x1f;
  int ss = (s>>11)&1;
  int sc = ((s+1024)>>11)&1;

  double sbh = Sn[is][0], sbl = Sn[is][1];
  double cbh = Sn[ic][0], cbl = Sn[ic][1];
  double slh = Sm[jm][0], sll = Sm[jm][1];
  double clh = Cm[jm][0], cll = Cm[jm][1];

  double ccl, cch = muldd_acc(clh,cll, cbh,cbl, &ccl);
  double ssl, ssh = muldd_acc(slh,sll, sbh,sbl, &ssl);
  double csl, csh = muldd_acc(clh,cll, sbh,sbl, &csl);
  double scl, sch = muldd_acc(slh,sll, cbh,cbl, &scl);

  double tcl, tch = fasttwosum(ssh, cch, &tcl);
  tcl += ccl + ssl;
  double tsl, tsh = fasttwosum(-sch,csh,&tsl); tsl += csl - scl;

  double tcl2; tch = fasttwosum(cbh,-tch, &tcl2); tcl = cbl - tcl + tcl2;
  double tsl2; tsh = fasttwosum(sbh,-tsh, &tsl2); tsl = sbl - tsl + tsl2;
  
  static const double sgn[2] = {0.0, -0.0};
  *ch = __builtin_copysign(1.0, sgn[sc])*tch;
  *cl = __builtin_copysign(1.0, sgn[sc])*tcl;

  *sh = __builtin_copysign(1.0, sgn[ss])*tsh;
  *sl = __builtin_copysign(1.0, sgn[ss])*tsl;
}
