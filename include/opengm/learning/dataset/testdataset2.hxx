#pragma once
#ifndef OPENGM_TESTDATASET2_HXX
#define OPENGM_TESTDATASET2_HXX

#include <vector>
#include <cstdlib>

#include <opengm/functions/learnable/lpotts.hxx>
#include <opengm/functions/learnable/sum_of_experts.hxx>

namespace opengm {
   namespace datasets{

      template<class GM>
      class TestDataset2{
      public:
         typedef GM                     GMType;
         typedef typename GM::ValueType ValueType;
         typedef typename GM::IndexType IndexType;
         typedef typename GM::LabelType LabelType; 
         typedef opengm::Parameters<ValueType,IndexType> ModelParameters;

         GM&                           getModel(const size_t i)  { return gms_[i]; }
         const std::vector<LabelType>& getGT(const size_t i)     { return gt_; }
         ModelParameters&              getModelParameters()      { return modelParameters_; }
         size_t                        getNumberOfParameters()   { return 3; }
         size_t                        getNumberOfModels()       { return gms_.size(); } 
         
         TestDataset2(size_t numModels=4); 

      private:
         std::vector<GM> gms_; 
         std::vector<LabelType> gt_; 
         ModelParameters modelParameters_;
      };
      


      template<class GM>
      TestDataset2<GM>::TestDataset2(size_t numModels)
         : modelParameters_(ModelParameters(3))
      {
         LabelType numberOfLabels = 2;
         gt_.resize(64*64,0);
         for(size_t i=32*64; i<64*64; ++i){
            gt_[i] = 1;
         }
         gms_.resize(numModels);
         for(size_t m=0; m<numModels; ++m){
            std::srand(m);
			for (int j = 0; j < 64*64; j++)
				gms_[m].addVariable(2);
            for(size_t y = 0; y < 64; ++y){ 
               for(size_t x = 0; x < 64; ++x) {
                  // function
                  const size_t numExperts = 2;
                  const std::vector<size_t> shape(1,numberOfLabels);
                  std::vector<marray::Marray<ValueType> > feat(numExperts,marray::Marray<ValueType>(shape.begin(), shape.end()));
                  ValueType val0 = (double)(gt_[y*64+x]) + (double)(std::rand()) / (double) (RAND_MAX) * 0.75 ;
                  feat[0](0) = std::fabs(val0-0);
                  feat[0](1) = std::fabs(val0-1); 
                  ValueType val1 = (double)(gt_[y*64+x]) + (double)(std::rand()) / (double) (RAND_MAX) * 1.5 ;
                  feat[1](0) = std::fabs(val1-0);
                  feat[1](1) = std::fabs(val1-1);
                  std::vector<size_t> wID(2);
                  wID[0]=1;  wID[1]=2;
                  opengm::functions::learnable::SumOfExperts<ValueType,IndexType,LabelType> f(shape,modelParameters_, wID, feat);
                  typename GM::FunctionIdentifier fid =  gms_[m].addFunction(f);

                  // factor
                  size_t variableIndices[] = {y*64+x};
                  gms_[m].addFactor(fid, variableIndices, variableIndices + 1);
               }
            }
          
            opengm::functions::learnable::LPotts<ValueType,IndexType,LabelType> f(modelParameters_,2,std::vector<size_t>(1,0),std::vector<ValueType>(1,1));
            typename GM::FunctionIdentifier fid = gms_[m].addFunction(f);      
            for(size_t y = 0; y < 64; ++y){ 
               for(size_t x = 0; x < 64; ++x) {
                  if(x + 1 < 64) { // (x, y) -- (x + 1, y)
                     size_t variableIndices[] = {y*64+x, y*64+x+1};
                     //sort(variableIndices, variableIndices + 2);
                     gms_[m].addFactor(fid, variableIndices, variableIndices + 2);
                  }
                  if(y + 1 < 64) { // (x, y) -- (x, y + 1)
                     size_t variableIndices[] = {y*64+x, (y+1)*64+x};
                     //sort(variableIndices, variableIndices + 2);
                     gms_[m].addFactor(fid, variableIndices, variableIndices + 2);
                  }
               }    
            }
         }
         
      };

   }
} // namespace opengm

#endif 
