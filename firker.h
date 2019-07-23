#ifndef FIRKER_H
#define FIRKER_H
#include <vector>
#include <map>
#include <functional>

class FirKer
{
public:
    FirKer();

    virtual bool calc() = 0;
    bool isValid() const;

    //bool setFrequency();
    bool setRank(int rank);
    int getRank(); //Warninig: rank doesn't have to match ker.size()
    bool setSamplingFreq(double freq);
    double getSampFreq() const;
    std::vector<double> transmission(int div) const;
    std::vector<double> transmission(int div, int beg, int end) const;
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
    bool setSpecification(const std::vector<double>& freqs, const std::vector<double>& gains);
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
    bool setSpecification(const std::vector<double>& freqs, const std::vector<double>& gains, const std::vector<double>& weights);
    bool calc();

protected:
    std::vector<double> freqs;
    std::vector<double> gains;
    std::vector<double> weights;

};



#endif //_FIRKER_H_
