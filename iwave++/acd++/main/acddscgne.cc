#include "acd_gfdm.h"

#include "state.hh"
#include "acd_sampler.hh"
#include "seamx_headers.hh"
#include "iwop.hh"
#include "samp.hh"
#include "sim.hh"
#include "pol.hh"
#include "blockop.hh"
#include "parserpp.hh"
#include "cgnealg.hh"

#include "CPsim.hh"

#ifdef IWAVE_USE_MPI
#include "mpisegypp.hh"
#include "mpigridpp.hh"
#else
#include "segypp.hh"
#include "gridpp.hh"
#endif
#include "segyops.hh"
#include "gridops.hh"

using namespace RVL;

using TSOpt::IWaveEnvironment;
using TSOpt::IWaveState;
using TSOpt::IWaveLinState;
using TSOpt::IWaveStep;
using TSOpt::IWaveStaticInit;
using TSOpt::IWaveDynamicInit;
using TSOpt::IWaveOp;
using TSOpt::Sampler;
using TSOpt::LinSampler;
using TSOpt::Sim;
using TSOpt::StdSim;
using TSOpt::StdRCSim;
using TSOpt::StdSimData;

using TSOpt::CPSim;

#ifdef IWAVE_USE_MPI
using TSOpt::MPIGridSpace;
using TSOpt::MPISEGYSpace;
#else
using TSOpt::GridSpace;
using TSOpt::SEGYSpace;
#endif

using TSOpt::SEGYLinMute;
using TSOpt::GridWindowOp;
using TSOpt::GridDerivOp;

using TSOpt::OpNewCreatePolicy;
using TSOpt::PolicyBase;
using RVL::StdProductSpace;
using RVL::Vector;
using RVL::Components;
using RVL::LinearOp;
using RVL::LinearOpFO;
using RVL::Operator;
using RVL::OpComp;
using RVL::OpFO;
using RVL::InjectOp;
using RVL::TensorOp;
using RVL::OperatorEvaluation;
using RVL::DerivEvaluation;
using RVL::RVLException;
using RVL::AssignFilename;
using RVL::AssignParams;
using RVL::parse;
using RVLUmin::CGNEAlg;

namespace ACD{

  /* dummy sampler policies to fill in the rest of the list */
  // class LinSamplerPolicy: public PolicyBase<IWaveState,LinSampler<IWaveLinState> > {};
  // class AdjSamplerPolicy: public PolicyBase<IWaveState,LinSampler<IWaveLinState> > {};
 
  /* Sampler Policies */
  /** FwdSamplerPolicy */
  typedef OpNewCreatePolicy<IWaveState,ACDSampler> ACDSamplerPolicy;
  /** LinSamplerPolicy */
  typedef OpNewCreatePolicy<IWaveLinState,ACDLinSampler> ACDLinSamplerPolicy;
  /** AdjSamplerPolicy */
  typedef OpNewCreatePolicy<IWaveLinState,ACDAdjSampler> ACDAdjSamplerPolicy;
 
  /* Sim Policies */  
  /** FwdSimPolicy */
  typedef OpNewCreatePolicy<StdSimData<IWaveState>, StdSim<IWaveState> > StdIWavePolicy;
  /** LinFwdSimPolicy */
  typedef OpNewCreatePolicy<StdSimData<IWaveState>, StdRCSim<IWaveState> > StdRCIWavePolicy; 
  /** AdjFwdSimPolicy */
  typedef OpNewCreatePolicy<StdSimData<IWaveState>, CPSim<IWaveState,TSIndex> > FwdCPSimPolicy;
  /** LinSimPolicy and AdjSimPolicy */
  typedef OpNewCreatePolicy<StdSimData<IWaveLinState>, StdSim<IWaveLinState> > LinSimPolicy;
 
  //typedef OpNewCreatePolicy<StdSimData<IWaveLinState>, StdSim<IWaveLinState> > AdjSimPolicy;
   
  /* dummy sim policies to fill in the rest of the list */
  //class LinFwdSimPolicy: public PolicyBase<StdSimData<IWaveState>, StdSim<IWaveState> > {};
  //class LinSimPolicy: public PolicyBase<StdSimData<IWaveLinState>, StdSim<IWaveLinState> > {};
  //class AdjFwdSimPolicy: public PolicyBase<StdSimData<IWaveState>, StdSim<IWaveState> > {};
  class AdjSimPolicy: public PolicyBase<StdSimData<IWaveLinState>, StdSim<IWaveLinState> > {};
}

char ** xargv;

int main(int argc, char ** argv) {

  using namespace ACD;
  try { 

    // put there to avoid forgetting
    if (argc<2) { 
      RVLException e;
      e<<"acdadj: adjoint action of linearized fwd map\n";
      e<<"usage: acdadj.x par=<par file>\n";
      throw e;
    }

    /* set up execution environment */
    int ts=0;
#ifdef IWAVE_USE_MPI
    MPI_Init_thread(&argc,&argv,MPI_THREAD_FUNNELED,&ts);
#endif    
    PARARRAY * pars = NULL;
    FILE * stream = NULL;
    IWaveEnvironment(argc,argv,ts,&pars,&stream);

#ifdef IWAVE_USE_MPI
    if (retrieveGroupID() == MPI_UNDEFINED) {
      fprintf(stream,"NOTE: finalize MPI, cleanup, exit\n");
    }
    else {
#endif

      /*********************************************************
       *             PARAMETER EXTRACTION                      *
       *********************************************************/

      /* files for acoustic params */
      string csqname = "";
      string buoyname = "";
      string dcsqname = "";
      string icsqname  = "";
      string hdrname = "";
      string trcname = "";
      string outfile = "";
      string dataest = "";
      string datares = "";
      string dsres   = "";
      parse<string>(*pars,"outfile",outfile);
      parse<string>(*pars,"dataest",dataest);
      parse<string>(*pars,"datares",datares);
      parse<string>(*pars,"dsres",dsres);
      parse_except<string>(*pars,"csq",csqname);
      parse_except<string>(*pars,"dcsq",dcsqname);
      parse_except<string>(*pars,"icsq",icsqname);
      parse_except<string>(*pars,"hdrfile",hdrname);
      parse_except<string>(*pars,"datafile",trcname);

      // assign window widths - default = 0;
      RPNT w;
      RASN(w,RPNT_0);
      parse<float>(*pars,"ww1",w[0]);
      parse<float>(*pars,"ww2",w[1]);
      parse<float>(*pars,"ww3",w[2]);

      float sm=0.0f;
      float wm=0.0f;
      float tm=0.0f;
      parse<float>(*pars,"mute_slope",sm);
      parse<float>(*pars,"mute_zotime",tm);
      parse<float>(*pars,"mute_width",wm);

      float rtol = 100.0*numeric_limits<float>::epsilon();
      float nrtol = 100.0*numeric_limits<float>::epsilon();
      int maxcount = 10;
      float maxstep = numeric_limits<float>::max();
      parse<float>(*pars,"ResidualTol",rtol);
      parse<float>(*pars,"GradientTol",nrtol);
      parse<int>(*pars,"MaxIter",maxcount);
      parse<float>(*pars,"MaxStep",maxstep);

      ireal alpha = REAL_ZERO;
      parse<ireal>(*pars,"DSParam",alpha);

      /*********************************************************
       *               INPUT CONSTRUCTION                      *
       *********************************************************/

      bool incore = true;
#ifdef IWAVE_USE_MPI
      MPIGridSpace msp(csqname, "csq", incore);
      MPIGridSpace dmsp(dcsqname, "dcsq", incore);
#else
      GridSpace msp(csqname, "csq", incore);
      GridSpace dmsp(dcsqname, "dcsq", incore);
#endif
      Vector<float> x(msp);
      Vector<float> dx(dmsp);

      /* make SEGY space and vector */
#ifdef IWAVE_USE_MPI
      MPISEGYSpace tsp(hdrname);
#else
      SEGYSpace tsp(hdrname);
#endif
    
      /* assign files */
      AssignFilename mfn(csqname);
      x.eval(mfn);

      AssignFilename dmfn(icsqname);
      dx.eval(dmfn);
    
      /*********************************************************
       *     OPERATOR CONSTRUCCTION, EVALUATION                *
       *********************************************************/

      /* simulator */
      IWaveOp<
      ACDSamplerPolicy,
	ACDLinSamplerPolicy,
	ACDAdjSamplerPolicy,
	StdIWavePolicy,
	StdRCIWavePolicy,
	LinSimPolicy,
	FwdCPSimPolicy,
	AdjSimPolicy
	> 
	iwop(msp,tsp,*pars,stream,acd_gfdm);

      GridWindowOp wop(dmsp,x,w);
      SEGYLinMute mute(sm,tm,wm);
      LinearOpFO<float> mop(tsp,tsp,mute,mute);
      OpComp<float> op(iwop,mop);
      int ddim = 0;
      if (retrieveGlobalRank() == 0) {
	if (msp.getGrid().gdim - msp.getGrid().dim < 1) {
	  RVLException e;
	  e<<"Error: acdds.scan.cc\n";
	  e<<"  reflectivity space does not have extra axis for DS\n";
	  throw e;
	}
	ddim=msp.getGrid().dim;
      }
      GridDerivOp ds(msp,ddim,alpha);

      TensorOp<ireal> dsop(op,ds);
      OpComp<ireal> gop(wop,dsop);

      // least squares target vector - [data,zero]^T
      Vector<ireal> dy(dsop.getRange());
      Components<ireal> cy(dy);
      AssignFilename tfn(trcname);
      cy[0].eval(tfn);
      cy[1].zero();
      
      /* work vector - displaced model */
      Vector<float> x0(gop.getDomain());
      x0.zero();

      /* evaluate at (displaced) origin */
      OperatorEvaluation<float> opeval(gop,x0);

      /* passed by reference, return with final values */
      float rnorm;
      float nrnorm;
      
      /* output stream */
      std::stringstream res;
      res<<scientific;

      res<<endl<<"*******************************************************"<<endl;
      res<<"* Acoustic Constant Density Linearized Inversion via";
      res<<"* Conjugate Gradient Algorithm for Normal Eqns"<<endl;
      res<<"* max iterations       = "<<maxcount<<endl;
      res<<"* residual tolerance   = "<<rtol<<endl;
      res<<"* normal res tolerance = "<<nrtol<<endl;
      res<<"* trust radius         = "<<maxstep<<endl;
      res<<"*******************************************************"<<endl;

      /* create CGNE object */
      CGNEAlg<float> alg(dx,opeval.getDeriv(),dy,
			 rnorm, nrnorm, rtol, nrtol, maxcount, maxstep, res);
      float nrnorm0=nrnorm;
      float rnorm0=rnorm;

      alg.run();
      
      Vector<ireal> dd(dsop.getRange());
      Components<ireal> cd(dd);
      if (dataest.size()) {
	AssignFilename afd(dataest);
	cd[0].eval(afd);
      }

      if (dsres.size()) {
	AssignFilename afds(dsres);
	cd[1].eval(afds);
      }

      opeval.getDeriv().applyOp(dx,dd);
      if (datares.size()) {
	Vector<float> dr(tsp);
	AssignFilename afr(datares);
	dr.eval(afr);
	dr.copy(cy[0]);
	dr.linComb(-1.0,cd[0]);
      }	
      
      // display results
      res<<"\n ******* summary ********  "<<endl;
      res<<"initial residual norm      = "<<rnorm0<<endl;
      res<<"residual norm              = "<<rnorm<<endl;
      res<<"residual redn              = "<<rnorm/rnorm0<<endl;
      res<<"initial gradient norm      = "<<nrnorm0<<endl;
      res<<"gradient norm              = "<<nrnorm<<endl;
      res<<"gradient redn              = "<<nrnorm/nrnorm0<<endl;
      res<<"ds residual norm           = "<<cd[1].norm()<<endl;

      if (retrieveRank() == 0) {
	if (outfile.size()>0) {
	  ofstream outf(outfile.c_str());
	  outf<<res.str();
	  outf.close();
	}
	else {
	  cout<<res.str();
	}
      }

      /*********************************************************
       *                       CLEANUP                         *
       *********************************************************/

      iwave_fdestroy();  

#ifdef IWAVE_USE_MPI
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
#endif  

  }
  catch (RVLException & e) {
    e<<"acdadj.x: ABORT on rk="<<retrieveGlobalRank()<<"\n";
#ifdef IWAVE_USE_MPI
    MPI_Abort(MPI_COMM_WORLD,0);
#endif
    exit(1);
  }
}
