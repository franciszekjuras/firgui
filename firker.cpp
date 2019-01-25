#include <algorithm>
#include <utility>
#include <cmath>
#include <vector>
#include <firpm/pm.h>
#include "firker.h"

//---> Base Filter Kernel Class <---//
FirKer::FirKer(){
    sampFreq = 1.;
}


void FirKer::invalidate(){this->valid = false; ker.clear();}

void FirKer::validate(){this->valid = true;}

bool FirKer::isValid() const {return this->valid;}

bool FirKer::setSampFreq(double freq){
    if(freq <= 0.)
        return false;
    this->sampFreq = freq;
    return true;
}

bool FirKer::setRank(int rank){
    if(rank < 0)
        return false;
    if(rank != this->rank){
        this->rank = rank;
        invalidate();
    }
    return true;
}

const std::vector<double>& FirKer::getKernel() const{return ker;}

double FirKer::getSampFreq() const{return sampFreq;}


std::vector<double> FirKer::transmission(int div) const{
    std::vector<double> trns;
    trns.resize(div+1);
    std::vector<double> luCos;
    luCos.resize(4*div);

    for(int k = 0; k < 4 * div; ++k)
        luCos[k] = std::cos(M_PI_2*static_cast<double>(k)/static_cast<double>(div));

    int mStart = ((ker.size()%2) == 0) ? 1 : 2;
    for(int i = 0; i <= div; ++i){
        int m = mStart;
        for(int j = ((ker.size()+1)/2); j < ker.size(); ++j){
            trns[i] += 2. * ker[j] * luCos[(i*m)%(4*div)];
            m += 2;
        }
    }
    if((ker.size()%2) == 1){
        for(int i = 0; i <= div; ++i)
            trns[i] += ker[ker.size()/2];
    }
    for(auto& t : trns){
        t = std::abs(t);
    }
    // for(int i = 0; i < trns.size(); ++i)
    // 	trns[i] = std::abs(trns[i]);

    return trns;
}

std::vector<double> FirKer::toBode(const std::vector<double>& trns){
    std::vector<double> bode(trns);
    for(auto& t : bode)
        t = 20.* std::log10(t);
    return bode;
}

//---> Least Squares Filter Kernel <---//

LeastSqFirKer::LeastSqFirKer(){
    gains.push_back(1.);
}


bool LeastSqFirKer::setSpecs(const std::vector<double>& freqs, const std::vector<double>& gains){
    if((freqs.size()+1) != gains.size())
        return false;
    //check for duplicates!
    if( freqs[0]<=0. || !std::is_sorted(freqs.begin(), freqs.end()) ||
     (freqs.end() != std::adjacent_find(freqs.begin(), freqs.end())) ||
     !std::all_of(gains.begin(),gains.end(),[](double x) {return x>=0.;}))
        return false;
    if(freqs != this->freqs || gains != this->gains){
        this->freqs = freqs; this->gains = gains;
        invalidate();
    }
    return true;
}

bool LeastSqFirKer::calc(){
    if(this->isValid())
        return true;
    //specification validation
    if(freqs.size()!=0 && freqs.back() >= (.5*sampFreq))
        return false;
    //body

    ker.resize(rank);
    double accg = 0.; auto gcrr = gains.begin();
    std::vector<std::pair<double,double>> spc;
    auto normFreqs = freqs;
    for(auto& f : normFreqs)
        f /= sampFreq;

    for(auto f : normFreqs){
        accg = *gcrr - *(++gcrr);
        if(accg != 0.)
            spc.push_back(std::make_pair((f), accg));
    }

    double t;
    if(rank%2 == 0){
        t = 0.5;
        if(gains.back()!=0.)
            spc.push_back(std::make_pair(0.5,gains.back()));
    }
    else{
        t = 1.;
        double acc = 0; auto g = gains.begin();
        for(auto f : normFreqs)
            acc += 2.*f*(*g - *(++g));
        acc += *g;
        ker[rank/2] = acc;
    }

    for(int k = ((rank - 1)/2)+1; k < rank; ++k){
        double acc = 0;
        for(auto s : spc)
            acc += s.second*std::sin(2.*M_PI*s.first*t);
        acc /= M_PI * t;
        t += 1.;
        ker[k] = acc;
    }

    for(int k = 0, j = rank - 1; k < rank/2;){
        ker[k++] = ker[j--];
    }


    this->validate();
    return true;
}


//---> Equiripple Filter Kernel <---//

EqRippleFirKer::EqRippleFirKer(){
    gains.push_back(1.); gains.push_back(1.);
    weights.push_back(1.);
}


bool EqRippleFirKer::setSpecs(const std::vector<double>& freqs, const std::vector<double>& gains, const std::vector<double>& weights){
    if((freqs.size()+2) != gains.size() || (weights.size()*2) != gains.size() )
        return false;

    if( freqs[0]<=0. || !std::is_sorted(freqs.begin(), freqs.end()) ||
     (freqs.end() != std::adjacent_find(freqs.begin(), freqs.end())) ||
     !std::all_of(gains.begin(),gains.end(),[](double x) {return x>=0.;}) ||
     !std::all_of(weights.begin(),weights.end(),[](double x) {return x>0.;}) )
        return false;

    if(freqs != this->freqs || gains != this->gains || weights != this->weights){
        this->freqs = freqs; this->gains = gains; this->weights = weights;
        invalidate();
    }
    return true;
}

bool EqRippleFirKer::calc(){
    if(this->isValid())
        return true;
    //specification validation
    if(freqs.size()!=0 && freqs.back() >= (.5*sampFreq))
        return false;
    //body

    ker.resize(rank);
    std::vector<double> normFreqs;
    normFreqs.resize(freqs.size()+2);
    for(int i = 0; i < freqs.size(); ++i)
        normFreqs[i+1] = freqs[i]*2./sampFreq; //firpm takes frequencies from 0 to 1!
    normFreqs.front() = 0.;
    normFreqs.back() = 1.;

    //calculate kernel
    PMOutput out = firpmRS(rank-1, normFreqs, gains, weights);

    if(std::isnan(out.Q) || std::isnan(out.delta) || (out.iter >= 101u))
        return false;

    this->ker = out.h;
    this->validate();
    return true;
}
