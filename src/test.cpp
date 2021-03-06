#include "PHD.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

void testCholesky () {
  cv::Matx<double, 5, 5> A1 = cv::Matx<double, 5, 5>::eye();
  cv::Matx<double, 5, 5> A2;
  A2 <<  1 , -1 , -1 , -1 , -1,
        -1 ,  2 ,  0 ,  0 ,  0,
        -1 ,  0 ,  3 ,  1 ,  1,
        -1 ,  0 ,  1 ,  4 ,  2,
        -1 ,  0 ,  1 ,  2 ,  5 ;
  cv::Matx<double, 8, 8> A3;
  A3 << 1, 1, 1, 1, 1, 1, 1, 1,
        1, 2, 3, 4, 5, 6, 7, 8,
        1, 3, 6, 10, 15, 21, 28, 36,
        1, 4, 10, 20, 35, 56, 84, 120,
        1, 5, 15, 35, 70, 126, 210, 330,
        1, 6, 21, 56, 126, 252, 462, 792,
        1, 7, 28, 84, 210, 462, 924, 1716,
        1, 8, 36, 120, 330, 792, 1716, 3432;
  cv::Matx<double, 5, 5> C1, C2;
  cv::Matx<double, 8, 8> C3;
  bool B1 = cholesky<5>(A1, C1);
  bool B2 = cholesky<5>(A2, C2);
  bool B3 = cholesky<8>(A3, C3);
  std::cout << A1 << std::endl << C1 << std::endl;
  std::cout << A2 << std::endl << C2 << std::endl;
  std::cout << A3 << std::endl << C3 << std::endl;
}

void testRandn() {
  cv::Vec<double, 4> mean; // = cv::Vec<double, 4>::zeros(); // FIXME
  mean << 0, 0, 0, 0; // Why does the above not work?
  cv::Matx<double, 4, 4> cov;
  cov << 2, 0.2, 0, 0,
       0.2, 1, 0, 0,
       0, 0, 0.7, -0.2,
       0, 0, -0.2, 1;
  std::vector<cv::Vec<double, 4> > elems =
    sampleMVGaussian<4> (mean, cov, 1000);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 1000; ++j) {
      std::cout << elems[j](i);
      if (j != 999) {std::cout << ",";}
    }
    std::cout << std::endl;
  }
}

void testMerge() {
  cv::Vec<double, 2> C1, C2;
  C1 << 0, 0;
  C2 << 10, 10;
  cv::Matx<double, 2, 2> P = 0.5*cv::Matx<double, 2, 2>::eye();
  std::vector<WeightedGaussian<2> > elements;
  WeightedGaussian<2> W1;
  WeightedGaussian<2> W2; 
  std::vector<cv::Vec<double, 2> > M1, M2;
  cv::Matx<double, 2, 2> P1, P2;
  for (int i = 0; i < 20; ++i) {
    // Create means and covariances
    M1 = sampleMVGaussian(C1, P, 1);
    M2 = sampleMVGaussian(C2, P, 1);
    P1 = cv::Matx<double, 2, 2>::zeros();
    P1 << 0.5 + cv::theRNG().gaussian(0.1), 0, 0.1+cv::theRNG().gaussian(0.1),
       0.6 + cv::theRNG().gaussian(0.05);
    P2 = cv::Matx<double, 2, 2>::zeros();
    P2 << 0.4 + cv::theRNG().gaussian(0.04), 0, 0.2+cv::theRNG().gaussian(0.1),
       0.7 + cv::theRNG().gaussian(0.05);
    P1 = 0.05*(P1 + P1.t());
    P2 = 0.05*(P2 + P2.t());
    W1 = WeightedGaussian<2>(1., M1[0], P1);
    W2 = WeightedGaussian<2>(1., M2[0], P2);
    elements.push_back(W1);
    elements.push_back(W2);
  }
  GaussianMixture<2> mixture(elements);
  std::ofstream out;
  out.open("bmerge.txt");
  for (int i = 0; i < mixture.size(); ++i) {
    out << mixture.at(i).getWeight() << ",";
    out << mixture.at(i).getMean()(0) << ",";
    out << mixture.at(i).getMean()(1) << ",";
    out << mixture.at(i).getCov()(0, 0) << ",";
    out << mixture.at(i).getCov()(0, 1) << ",";
    out << mixture.at(i).getCov()(1, 0) << ",";
    out << mixture.at(i).getCov()(1, 1) << "\n";
  }
  out.close();
  mixture.merge(2);
  out.open("amerge.txt");
  for (int i = 0; i < mixture.size(); ++i) {
    out << mixture.at(i).getWeight() << ",";
    out << mixture.at(i).getMean()(0) << ",";
    out << mixture.at(i).getMean()(1) << ",";
    out << mixture.at(i).getCov()(0, 0) << ",";
    out << mixture.at(i).getCov()(0, 1) << ",";
    out << mixture.at(i).getCov()(1, 0) << ",";
    out << mixture.at(i).getCov()(1, 1) << "\n";
  }
}

std::vector<cv::Vec<double, 2> > simMeasurements(
    std::vector<cv::Vec<double, 2> > GT) {
  std::vector<cv::Vec<double, 2> > sim;
  cv::Matx<double, 2, 2> measCov = (0.01)*cv::Matx<double, 2, 2>::eye();
  double Pd = 0.9;
  // Generate clutter
  int nc = samplePoisson(0.1);
  cv::Vec<double, 2> clutter;
  for (int i = 0; i < nc; ++i) {
    std::cout << "*";
    clutter(0) = 10*cv::randu<double>();
    clutter(1) = 10*cv::randu<double>();
    sim.push_back(clutter);
  }
  // Generate measurements
  std::vector<cv::Vec<double, 2> >::iterator it;
  for (it = GT.begin(); it != GT.end(); ++it) {
    if (cv::randu<double>() < Pd) {
      sim.push_back(sampleMVGaussian<2>(*it, measCov, 1).back());
    }
  }
  return sim;
}

void testSim(){
  std::vector<cv::Vec<double, 2> > GT;
  cv::Vec<double, 2> C;
  C << 5, 5;
  GT.push_back(C);
  std::vector<cv::Vec<double, 2> > meas;
  while (cv::waitKey(1000) != 'q') {
    meas.clear();
    meas = simMeasurements(GT);
    for (int i = 0; i < meas.size(); ++i) {
      std::cout << meas[i] << std::endl;
    }
    std::cout << "---" << std::endl;
  }
}

// TODO TEST
std::vector<std::vector<cv::Vec<double, 2> > >
readMeasurements(std::string filename) {
  std::ifstream inputFile;
  std::string line;
  std::string measurementString;
  std::stringstream splitLine;
  std::stringstream splitMeasurement;
  std::string strX, strY;
  double measurementX, measurementY;
  cv::Vec<double, 2> measurement;
  std::vector<cv::Vec<double, 2> > frame;
  std::vector<std::vector<cv::Vec<double, 2> > > results;
  inputFile.open(filename.c_str());
  while (std::getline(inputFile, line)) {
    frame.clear();
    splitLine.clear();
    splitLine.str(line);
    //splitLine.seekg(0);
    while(std::getline(splitLine, measurementString, '|')) {
      splitMeasurement.str(measurementString);
      //splitMeasurement.seekg(0);
      splitMeasurement.clear();
      std::getline(splitMeasurement, strX, ',');
      std::getline(splitMeasurement, strY, ',');
      measurementX = atof(strX.c_str());
      measurementY = atof(strY.c_str());
      frame.push_back(cv::Vec<double, 2>(measurementX, measurementY));
    }
    results.push_back(frame);
  }
  return results;
}

void printMeasurements(std::vector<std::vector<cv::Vec<double, 2> > > meas) {
  std::vector<std::vector<cv::Vec<double, 2> > >::iterator it;
  std::vector<cv::Vec<double, 2> >::iterator jt;
  int frameNo = 0;
  for (it = meas.begin(); it != meas.end(); ++it) {
    std::cout << "[" << frameNo++ << "] ";
    for (jt = it->begin(); jt != it->end(); ++jt) {
      std::cout << "(" << (*jt)(0) << "," << (*jt)(1) << ") ";
    }
    std::cout << std::endl;
  }
}

void writeGM4(std::vector<GaussianMixture<4> > data, std::string filename) {
  // Write the weight, mean, and lower diagonal of the covariance matrix
  // of a vector of GaussianMixtures of dimension 4
  std::vector<GaussianMixture<4> >::iterator it;
  std::vector<WeightedGaussian<4> >::iterator jt;
  std::ofstream file(filename.c_str());
  for(it = data.begin(); it != data.end(); ++it) {
    if(it != data.begin()) {
      file << "===\n";
    }
    for(jt = it->mComponents.begin(); jt != it->mComponents.end(); ++jt) {
      file << jt->getWeight() << "," << jt->getMean()(0) << "," <<
        jt->getMean()(1) << "," << jt->getMean()(2) << "," << jt->getMean()(3)
        << "," << 
        jt->getCov()(0, 0) << "," << jt->getCov()(1, 0) << "," <<
        jt->getCov()(2, 0) << "," << jt->getCov()(3, 0) << "," << 
        jt->getCov()(1, 1) << "," << jt->getCov()(2, 1) << "," <<
        jt->getCov()(3, 1) << "," << jt->getCov()(2, 2) << "," <<
        jt->getCov()(3, 2) << "," << jt->getCov()(3, 3) << "\n";
    }
  }
  file.close();
}

void writeGM2(std::vector<GaussianMixture<2> > data, std::string filename) {
  // Write the weight, mean, and lower diagonal of the covariance matrix
  // of a vector of GaussianMixtures of dimension 2
  std::vector<GaussianMixture<2> >::iterator it;
  std::vector<WeightedGaussian<2> >::iterator jt;
  std::ofstream file(filename.c_str());
  for(it = data.begin(); it != data.end(); ++it) {
    if(it != data.begin()) {
      file << "===\n";
    }
    for(jt = it->mComponents.begin(); jt != it->mComponents.end(); ++jt) {
      file << jt->getWeight() << "," << jt->getMean()(0) << "," <<
        jt->getMean()(1) << "," << jt->getCov()(0, 0) << "," <<
        jt->getCov()(1, 0) << "," << jt->getCov()(1, 1) <<  "\n";
    }
  }
  file.close();
}

GMPHDFilterParams readFilterParams(std::string filename) {
  std::ifstream inputFile(filename.c_str());
  double probSurvival, probDetection, clutterDensity, mergeThreshold,
         trimThreshold;
  int truncThreshold;
  std::string line;
  inputFile >> probSurvival;
  inputFile >> probDetection;
  inputFile >> clutterDensity;
  inputFile >> mergeThreshold;
  inputFile >> trimThreshold;
  inputFile >> truncThreshold;
  inputFile.close();
  return GMPHDFilterParams(probSurvival, probDetection, clutterDensity,
      mergeThreshold, trimThreshold, truncThreshold);
}

void testCvPHD(){
  cv::Matx<double, 4, 4> dynMatrix;
  cv::Matx<double, 4, 4> procNoise;
  cv::Matx<double, 2, 4> measMatrix;
  cv::Matx<double, 2, 2> measNoise;
  cv::Matx<double, 4, 4> newElemCov;
  double dt = 1;
  dynMatrix << 1, 0, dt,  0,
               0, 1,  0, dt,
               0, 0,  1,  0,
               0, 0,  0,  1;
  procNoise << 0.25,   0,    0,    0,
                 0, 0.25,    0,    0,
                 0,   0, 0.02,    0,
                 0,   0,    0, 0.02;
  measMatrix << 1, 0, 0, 0,
                0, 1, 0, 0;
  measNoise << 0.25, 0, 0, 0.25;
  newElemCov << 0.25, 0, 0, 0,
                0, 0.25, 0, 0,
                0, 0, 0.1, 0,
                0, 0, 0, 0.1;
  LinearMotionModel<4> * CVMotionModel = 
    new LinearMotionModel<4> (dynMatrix, procNoise);
  LinearMeasurementModel<4, 2> * PMeasurementModel = 
    new LinearMeasurementModel<4, 2>(measMatrix, measNoise, newElemCov);
  GMPHDFilterParams filterParams = readFilterParams("params.txt");
  GMPHDFilter<4, 2> filter(CVMotionModel, PMeasurementModel, filterParams);
  std::vector<std::vector<cv::Vec<double, 2> > > measurements = 
    readMeasurements("../../Matlab/measurements.txt");
  std::vector<std::vector<cv::Vec<double, 2> > >::iterator it;
  std::vector<cv::Vec<double, 4> > stateEstimate;
  std::vector<GaussianMixture<4> > intensities;
  double estimatedCardinality;
  for (it = measurements.begin(); it != measurements.end(); ++it) {
    filter.predict();
    filter.update(*it);
    std::cout << "(" << it->size() << ") " << filter.getPHD().size() << std::endl;
    intensities.push_back(filter.getPHD());
    stateEstimate = filter.getStateEstimate();
    estimatedCardinality = 0.;
    for (int i = 0; i < filter.getPHD().size(); ++i) {
      estimatedCardinality += filter.getPHD().at(i).getWeight();
    }
  }
  writeGM4(intensities, "gm.txt");
}

void testCpPHD(){
  cv::Matx<double, 2, 2> dynMatrix;
  cv::Matx<double, 2, 2> procNoise;
  cv::Matx<double, 2, 2> measMatrix;
  cv::Matx<double, 2, 2> measNoise;
  cv::Matx<double, 2, 2> newElemCov;
  dynMatrix = cv::Matx<double, 2, 2>::eye();
  procNoise << 0.25, 0, 0, 0.25;
  measMatrix = cv::Matx<double, 2, 2>::eye();
  measNoise << 0.2, 0, 0, 0.2;
  newElemCov << 0.5, 0, 0, 0.5;
  LinearMotionModel<2> * CVMotionModel = 
    new LinearMotionModel<2>(dynMatrix, procNoise);
  LinearMeasurementModel<2, 2> * PMeasurementModel = 
    new LinearMeasurementModel<2, 2> (measMatrix, measNoise, newElemCov);
  GMPHDFilterParams filterParams = readFilterParams("params.txt");
  GMPHDFilter<2, 2> filter(CVMotionModel, PMeasurementModel, filterParams);
  std::vector<std::vector<cv::Vec<double, 2> > > measurements = 
    readMeasurements("../../Matlab/measurements.txt");
  std::vector<std::vector<cv::Vec<double, 2> > >::iterator it;
  std::vector<GaussianMixture<2> > intensities;
  double estimatedCardinality;
  unsigned int iteration = 0;
  for (it = measurements.begin(); it != measurements.end(); ++it) {
    filter.predict();
    filter.update(*it);
    estimatedCardinality = 0.;
    for (int i = 0; i < filter.getPHD().size(); ++i) {
      estimatedCardinality += filter.getPHD().at(i).getWeight();
    }
    std::cout << iteration++ << ": (" << it->size() << ") "
      << filter.getPHD().size() << " ~ " << estimatedCardinality << "("
      << filter.getMultiObjectLikelihood() << ")" << std::endl;
    intensities.push_back(filter.getPHD());
  }
  writeGM2(intensities, "gm.txt");
}

void testParticleFilter() {
  cv::Matx<double, 2, 2> dynMatrix;
  cv::Matx<double, 2, 2> procNoise;
  cv::Matx<double, 2, 2> measMatrix;
  cv::Matx<double, 2, 2> measNoise;
  cv::Matx<double, 2, 2> newElemCov;
  cv::Matx<double, 2, 2> particleCov;
  dynMatrix = cv::Matx<double, 2, 2>::eye();
  procNoise << 0.001, 0, 0, 0.001;
  measMatrix = cv::Matx<double, 2, 2>::eye();
  measNoise << 0.01, 0, 0, 0.01;
  newElemCov << 0.001, 0, 0, 0.001;
  particleCov << 0.1, 0, 0, 0.1;
  LinearMotionModel<2> CVMotionModel(dynMatrix, procNoise);
  LinearMeasurementModel<2, 2> PMeasurementModel(measMatrix, measNoise,
      newElemCov);
  GMPHDFilterParams filterParams = readFilterParams("params.txt");
  CPPHDParticleFilter<2, 2> filter(400, particleCov, &CVMotionModel,
     &PMeasurementModel, filterParams);
  std::vector<std::vector<cv::Vec<double, 2> > > measurements = 
    readMeasurements("../../Matlab/measurements.txt");
  std::vector<std::vector<cv::Vec<double, 2> > >::iterator it;
  std::vector<GaussianMixture<2> > intensities;
  GMPHDFilter<2, 2> bestFilter;
  std::vector<cv::Vec<double, 2> > estimatedBias;
  double estimatedCardinality;
  unsigned int iteration = 0;
  for (it = measurements.begin(); it != measurements.end(); ++it) {
    if (iteration > 0) {
      filter.predict();
    }
    if (iteration > 0) {
      filter.resample(*it);
    }
    filter.update(*it);
    bestFilter = std::max_element(
        filter.mBelief.begin(), filter.mBelief.end() )->mPHDFilter;
    estimatedBias.push_back(std::max_element(
        filter.mBelief.begin(), filter.mBelief.end())->mBias) ;
    estimatedCardinality = 0.;
    for (int i = 0; i < bestFilter.getPHD().size(); ++i) {
      estimatedCardinality += bestFilter.getPHD().at(i).getWeight();
    }
    std::cout << iteration++ << ": (" << it->size() << ") "
      << bestFilter.getPHD().size() << " ~ " << estimatedCardinality << "("
      << bestFilter.getMultiObjectLikelihood() << ")" << std::endl;
    intensities.push_back(bestFilter.getPHD());
  }
  writeGM2(intensities, "gm.txt");
  // Quick + dirty write biases
  std::ofstream biasFile("biases.txt");
  for (int i = 0; i < estimatedBias.size(); ++i) {
    biasFile << estimatedBias[i][0] << "," << estimatedBias[i][1] << "\n";
  }
  biasFile.close();
}

void testPredictMOL() {
  cv::Matx<double, 2, 2> dynMatrix;
  cv::Matx<double, 2, 2> procNoise;
  cv::Matx<double, 2, 2> measMatrix;
  cv::Matx<double, 2, 2> measNoise;
  cv::Matx<double, 2, 2> newElemCov;
  cv::Matx<double, 2, 2> particleCov;
  dynMatrix = cv::Matx<double, 2, 2>::eye();
  procNoise << 0.1, 0, 0, 0.1;
  measMatrix = cv::Matx<double, 2, 2>::eye();
  measNoise << 0.2, 0, 0, 0.2;
  newElemCov << 1, 0, 0, 1;
  particleCov << 0.2, 0, 0, 0.2;
  LinearMotionModel<2> * CVMotionModel =
    new LinearMotionModel<2>(dynMatrix, procNoise);
  LinearMeasurementModel<2, 2> * PMeasurementModel =
    new LinearMeasurementModel<2, 2>(measMatrix, measNoise, newElemCov);
  GMPHDFilterParams filterParams = readFilterParams("params.txt");
  GMPHDFilter<2, 2> filter(CVMotionModel, PMeasurementModel, filterParams);
  std::vector<std::vector<cv::Vec<double, 2> > > measurements = 
    readMeasurements("../../Matlab/measurements.txt");
  std::vector<std::vector<cv::Vec<double, 2> > >::iterator it;
  double predictedMOL;
  unsigned int iteration = 0;
  for (it = measurements.begin(); it != measurements.end(); ++it) {
    if (iteration > 0) {
      filter.predict();
    }
    predictedMOL = filter.predictMeasurementLikelihood(*it);
    filter.update(*it);
    std::cout << "[" << iteration++ << "] Difference: " <<
      predictedMOL - filter.getMultiObjectLikelihood() << "\n";
  }
}

int main() {
//  std::vector<std::vector<cv::Vec<double, 2> > > measurements = 
//    readMeasurements("./meas.txt");
//  printMeasurements(measurements);
  //testParticleFilter();
  testParticleFilter();
  return 0;
}
