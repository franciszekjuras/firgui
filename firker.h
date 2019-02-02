#ifndef _FIRKER_H_
#define _FIRKER_H_
#include <vector>
#include <map>
#include <functional>

class FirKer
{
public:
    FirKer();

    virtual bool calc(){return false;}
    bool isValid() const;

    //bool setFrequency();
    bool setRank(int rank);
    bool setSampFreq(double freq);
    double getSampFreq() const;
    std::vector<double> transmission(int div) const;
    static std::vector<double> toBode(const std::vector<double>& trns);
    const std::vector<double>& getKernel() const;

protected:
    bool valid;
    int rank;
    std::vector<double> ker;
    double sampFreq;
    void invalidate();
    void validate();

};


class LeastSqFirKer : public FirKer
{
public:
    enum class Window {
        none,
        hamming,
        blackman
    };

    LeastSqFirKer();
    bool setSpecs(const std::vector<double>& freqs, const std::vector<double>& gains);
    void setWindow(Window wnd);
    bool calc();

protected:
    std::vector<double> freqs;
    std::vector<double> gains;
    Window wnd;
    std::vector<double> kerNoWin;
    std::map<Window, std::function<double(const double&)>> windows;

};

class EqRippleFirKer : public FirKer
{
public:
    EqRippleFirKer();
    bool setSpecs(const std::vector<double>& freqs, const std::vector<double>& gains, const std::vector<double>& weights);
    bool calc();

protected:
    std::vector<double> freqs;
    std::vector<double> gains;
    std::vector<double> weights;

};



#endif //_FIRKER_H_
