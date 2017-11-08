#ifndef CombineTools_Process_h
#define CombineTools_Process_h
#include <memory>
#include <string>
#include "TH1.h"
#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooAbsData.h"
#include "CombineHarvester/CombineTools/interface/MakeUnique.h"
#include "CombineHarvester/CombineTools/interface/Object.h"

namespace ch {

class Process : public Object {
 public:
  Process();
  ~Process();
  Process(Process const& other);
  Process(Process&& other);
  Process& operator=(Process other);


  void set_rate(double const& rate) { rate_ = rate; }
  double rate() const { return norm_ ? norm_->getVal() * rate_ : rate_; }

  /**
   * Get the process normalisation **without** multiplying by the RooAbsReal
   * value (in the case that it's present)
   *
   * Generally this isn't a very useful method, it just returns the value of
   * the `rate` class member without multipling by the RooAbsReal term. If the
   * process has a RooAbsReal attached then this is often an (or the)
   * important part of determining the total process normalisation. One place
   * this is useful is writing the rate into the text datacard.
   */
  double no_norm_rate() const { return rate_; }

  void set_shape(std::unique_ptr<TH1> shape, bool set_rate);

  void set_shape(TH1 const& shape, bool set_rate);

  TH1 const* shape() const { return shape_.get(); }

  std::unique_ptr<TH1> ClonedShape() const;
  std::unique_ptr<TH1> ClonedScaledShape() const;

  TH1F ShapeAsTH1F() const;

  void set_pdf(RooAbsPdf* pdf) { pdf_ = pdf; }
  RooAbsPdf const* pdf() const { return pdf_; }

  void set_data(RooAbsData* data) { data_ = data; }
  RooAbsData const* data() const { return data_; }

  void set_norm(RooAbsReal* norm) { norm_ = norm; }
  RooAbsReal const* norm() const { return norm_; }

  friend std::ostream& operator<< (std::ostream &out, Process const& val);
  static std::ostream& PrintHeader(std::ostream &out);

 private:
  double rate_;
  std::unique_ptr<TH1> shape_;
  RooAbsPdf* pdf_;
  RooAbsData* data_;
  RooAbsReal* norm_;

  friend void swap(Process& first, Process& second);
};
}

#endif
