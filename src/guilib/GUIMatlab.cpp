#ifdef MATLAB
#include <stdio.h>
#include <string.h>

#include "lib/io.h"
#include "mex.h"
#include "distributions/hmm/HMM.h"
#include "classifier/svm/SVM.h"
#include "features/Labels.h"
#include "features/RealFeatures.h"
#include "kernel/WeightedDegreeCharKernel.h"

#include "guilib/GUIMatlab.h"
#include "gui/TextGUI.h"
#include "gui/GUI.h"

extern CTextGUI* gui;

CGUIMatlab::CGUIMatlab()
{
}

bool CGUIMatlab::send_command(CHAR* cmd)
{
	return (gui->parse_line(cmd));
}

bool CGUIMatlab::get_hmm(mxArray* retvals[])
{
	CHMM* h=gui->guihmm.get_current();

	if (h)
	{
	  fprintf(stderr, "N=%i  M=%i\n", h->get_N(), h->get_M()) ;
		mxArray* mx_p=mxCreateDoubleMatrix(1, h->get_N(), mxREAL);
		mxArray* mx_q=mxCreateDoubleMatrix(1, h->get_N(), mxREAL);
		mxArray* mx_a=mxCreateDoubleMatrix(h->get_N(), h->get_N(), mxREAL);
		mxArray* mx_b=mxCreateDoubleMatrix(h->get_N(), h->get_M(), mxREAL);

		if (mx_p && mx_q && mx_a && mx_b)
		{
			double* p=mxGetPr(mx_p);
			double* q=mxGetPr(mx_q);
			double* a=mxGetPr(mx_a);
			double* b=mxGetPr(mx_b);

			int i,j;
			for (i=0; i< h->get_N(); i++)
			{
				p[i]=h->get_p(i);
				q[i]=h->get_q(i);
			}

			for (i=0; i< h->get_N(); i++)
				for (j=0; j< h->get_N(); j++)
					a[i+j*h->get_N()]=h->get_a(i,j);

			for (i=0; i< h->get_N(); i++)
				for (j=0; j< h->get_M(); j++)
					b[i+j*h->get_N()]=h->get_b(i,j);

			retvals[0]=mx_p;
			retvals[1]=mx_q;
			retvals[2]=mx_a;
			retvals[3]=mx_b;

			return true;
		}
	}

	return false;
}

bool CGUIMatlab::append_hmm(const mxArray* vals[])
{
	CHMM* old_h=gui->guihmm.get_current();
	assert(old_h);

	const mxArray* mx_p=vals[1];
	const mxArray* mx_q=vals[2];
	const mxArray* mx_a=vals[3];
	const mxArray* mx_b=vals[4];

	INT N=mxGetN(mx_a);
	INT M=mxGetN(mx_b);

	if (mx_p && mx_q && mx_a && mx_b)
	{
		CHMM* h=new CHMM(N, M, NULL,
				gui->guihmm.get_pseudo(), gui->guihmm.get_number_of_tables());
		if (h)
		{
			CIO::message(M_INFO, "N:%d M:%d p:(%d,%d) q:(%d,%d) a:(%d,%d) b(%d,%d)\n",
					N, M,
					mxGetN(mx_p), mxGetM(mx_p), 
					mxGetN(mx_q), mxGetM(mx_q), 
					mxGetN(mx_a), mxGetM(mx_a), 
					mxGetN(mx_b), mxGetM(mx_b));
			if (
					mxGetN(mx_p) == h->get_N() && mxGetM(mx_p) == 1 &&
					mxGetN(mx_q) == h->get_N() && mxGetM(mx_q) == 1 &&
					mxGetN(mx_a) == h->get_N() && mxGetM(mx_a) == h->get_N() &&
					mxGetN(mx_b) == h->get_M() && mxGetM(mx_b) == h->get_N()
			   )
			{
				double* p=mxGetPr(mx_p);
				double* q=mxGetPr(mx_q);
				double* a=mxGetPr(mx_a);
				double* b=mxGetPr(mx_b);

				int i,j;
				for (i=0; i< h->get_N(); i++)
				{
					h->set_p(i, p[i]);
					h->set_q(i, q[i]);
				}

				for (i=0; i< h->get_N(); i++)
					for (j=0; j< h->get_N(); j++)
						h->set_a(i,j, a[i+j*h->get_N()]);

				for (i=0; i< h->get_N(); i++)
					for (j=0; j< h->get_M(); j++)
						h->set_b(i,j, b[i+j*h->get_N()]);

				CIO::message(M_INFO, "h %d , M: %d\n", h, h->get_M());

				old_h->append_model(h);

				delete h;

				return true;
			}
			else
				CIO::message(M_ERROR, "model matricies not matching in size\n");
		}
	}
	return false;
}

bool CGUIMatlab::set_hmm(const mxArray* vals[])
{
	const mxArray* mx_p=vals[1];
	const mxArray* mx_q=vals[2];
	const mxArray* mx_a=vals[3];
	const mxArray* mx_b=vals[4];

	INT N=mxGetN(mx_a);
	INT M=mxGetN(mx_b);

	CHMM* h=new CHMM(N, M, NULL,
			gui->guihmm.get_pseudo(), gui->guihmm.get_number_of_tables());

	if ( mx_p && mx_q && mx_a && mx_b)
	{

		if (h)
		{
			CIO::message(M_ERROR, "N:%d M:%d p:(%d,%d) q:(%d,%d) a:(%d,%d) b(%d,%d)\n",
					N, M,
					mxGetN(mx_p), mxGetM(mx_p), 
					mxGetN(mx_q), mxGetM(mx_q), 
					mxGetN(mx_a), mxGetM(mx_a), 
					mxGetN(mx_b), mxGetM(mx_b));

			if (
					mxGetN(mx_p) == h->get_N() && mxGetM(mx_p) == 1 &&
					mxGetN(mx_q) == h->get_N() && mxGetM(mx_q) == 1 &&
					mxGetN(mx_a) == h->get_N() && mxGetM(mx_a) == h->get_N() &&
					mxGetN(mx_b) == h->get_M() && mxGetM(mx_b) == h->get_N()
			   )
			{
				double* p=mxGetPr(mx_p);
				double* q=mxGetPr(mx_q);
				double* a=mxGetPr(mx_a);
				double* b=mxGetPr(mx_b);

				int i,j;
				for (i=0; i< h->get_N(); i++)
				{
					h->set_p(i, p[i]);
					h->set_q(i, q[i]);
				}

				for (i=0; i< h->get_N(); i++)
					for (j=0; j< h->get_N(); j++)
						h->set_a(i,j, a[i+j*h->get_N()]);

				for (i=0; i< h->get_N(); i++)
					for (j=0; j< h->get_M(); j++)
						h->set_b(i,j, b[i+j*h->get_N()]);

				gui->guihmm.set_current(h);
				return true;
			}
			else
				CIO::message(M_ERROR, "model matricies not matching in size\n");
		}
	}

	return false;
}


bool CGUIMatlab::best_path_no_b(const mxArray* vals[], mxArray* retvals[])
{
	const mxArray* mx_p=vals[1];
	const mxArray* mx_q=vals[2];
	const mxArray* mx_a=vals[3];
	const mxArray* mx_max_iter=vals[4];

	INT max_iter = (INT)mxGetScalar(mx_max_iter) ;
	
	INT N=mxGetN(mx_a);

	if ( mx_p && mx_q && mx_a)
	{
		if (
			mxGetN(mx_p) == N && mxGetM(mx_p) == 1 &&
			mxGetN(mx_q) == N && mxGetM(mx_q) == 1 &&
			mxGetN(mx_a) == N && mxGetM(mx_a) == N
			)
		{
			double* p=mxGetPr(mx_p);
			double* q=mxGetPr(mx_q);
			double* a=mxGetPr(mx_a);
			
			CHMM* h=new CHMM(N, p, q, a);
			
			INT *my_path = new INT[max_iter] ;
			int best_iter = 0 ;
			REAL prob = h->best_path_no_b(max_iter, best_iter, my_path) ;

			mxArray* mx_prob = mxCreateDoubleMatrix(1, 1, mxREAL);
			double* p_prob = mxGetPr(mx_prob);
			p_prob[0] = prob ;
			
			mxArray* mx_result = mxCreateDoubleMatrix(1, best_iter+1, mxREAL);
			double* result = mxGetPr(mx_result);
			for (INT i=0; i<best_iter+1; i++)
				result[i]=my_path[i] ;
			
			retvals[0]=mx_prob ;
			retvals[1]=mx_result ;
			delete h ;
			
			return true;
		}
		else
			CIO::message(M_ERROR, "model matricies not matching in size\n");
	}

	return false;
}

bool CGUIMatlab::best_path_no_b_trans(const mxArray* vals[], mxArray* retvals[])
{
	const mxArray* mx_p=vals[1];
	const mxArray* mx_q=vals[2];
	const mxArray* mx_a_trans=vals[3];
	const mxArray* mx_max_iter=vals[4];
	const mxArray* mx_nbest=vals[5];

	INT max_iter = (INT)mxGetScalar(mx_max_iter) ;
	INT nbest    = (INT)mxGetScalar(mx_nbest) ;
	if (nbest<1)
		return false ;
	if (max_iter<1)
		return false ;
	
	INT N=mxGetN(mx_p);

	if ( mx_p && mx_q && mx_a_trans)
	{
		if (
			mxGetN(mx_p) == N && mxGetM(mx_p) == 1 &&
			mxGetN(mx_q) == N && mxGetM(mx_q) == 1 &&
			mxGetN(mx_a_trans) == 3
			)
		{
			double* p=mxGetPr(mx_p);
			double* q=mxGetPr(mx_q);
			double* a=mxGetPr(mx_a_trans);
			
			CHMM* h=new CHMM(N, p, q, mxGetM(mx_a_trans), a);
			
			INT *my_path = new INT[(max_iter+1)*nbest] ;
			memset(my_path, -1, (max_iter+1)*nbest*sizeof(INT)) ;
			
			int max_best_iter = 0 ;
			mxArray* mx_prob = mxCreateDoubleMatrix(1, nbest, mxREAL);
			double* p_prob = mxGetPr(mx_prob);
			
			h->best_path_no_b_trans(max_iter, max_best_iter, nbest, p_prob, my_path) ;

			mxArray* mx_result=mxCreateDoubleMatrix(nbest, max_best_iter+1, mxREAL);
			double* result=mxGetPr(mx_result);
			
			for (INT k=0; k<nbest; k++)
			{
				for (INT i=0; i<max_best_iter+1; i++)
					result[i*nbest+k] = my_path[i+k*(max_iter+1)] ;
			}
			 
			
			retvals[0]=mx_prob ;
			retvals[1]=mx_result ;
			
			delete h ;
			delete[] my_path ;
			return true;
		}
		else
			CIO::message(M_ERROR, "model matricies not matching in size\n");
	}

	return false;
}


bool CGUIMatlab::model_prob_no_b_trans(const mxArray* vals[], mxArray* retvals[])
{
	const mxArray* mx_p=vals[1];
	const mxArray* mx_q=vals[2];
	const mxArray* mx_a_trans=vals[3];
	const mxArray* mx_max_iter=vals[4];

	INT max_iter = (INT)mxGetScalar(mx_max_iter) ;
	if (max_iter<1)
		return false ;
	
	INT N=mxGetN(mx_p);

	if ( mx_p && mx_q && mx_a_trans)
	{
		if (
			mxGetN(mx_p) == N && mxGetM(mx_p) == 1 &&
			mxGetN(mx_q) == N && mxGetM(mx_q) == 1 &&
			mxGetN(mx_a_trans) == 3
			)
		{
			double* p=mxGetPr(mx_p);
			double* q=mxGetPr(mx_q);
			double* a=mxGetPr(mx_a_trans);
			
			CHMM* h=new CHMM(N, p, q, mxGetM(mx_a_trans), a);
			
			mxArray* mx_prob = mxCreateDoubleMatrix(1, max_iter, mxREAL);
			double* p_prob = mxGetPr(mx_prob);
			
			h->model_prob_no_b_trans(max_iter, p_prob) ;

			retvals[0]=mx_prob ;
			
			delete h ;
			return true;
		}
		else
			CIO::message(M_ERROR, "model matricies not matching in size\n");
	}

	return false;
}



bool CGUIMatlab::hmm_classify(mxArray* retvals[])
{
	CFeatures* f=gui->guifeatures.get_test_features();
	if (f)
	{
		int num_vec=f->get_num_vectors();

		mxArray* mx_result=mxCreateDoubleMatrix(1, num_vec, mxREAL);
		double* result=mxGetPr(mx_result);
		CLabels* l=gui->guihmm.classify();

		for (int i=0; i<num_vec; i++)
			result[i]=l->get_label(i);

		delete l;

		retvals[0]=mx_result;
		return true;
	}
	return false;
}

bool CGUIMatlab::hmm_classify_example(mxArray* retvals[], int idx)
{
	mxArray* mx_result=mxCreateDoubleMatrix(1, 1, mxREAL);
	double* result=mxGetPr(mx_result);
	*result=gui->guihmm.classify_example(idx);
	retvals[0]=mx_result;
	return true;
}

bool CGUIMatlab::one_class_hmm_classify(mxArray* retvals[], bool linear)
{
	CFeatures* f=gui->guifeatures.get_test_features();
	if (f)
	{
		int num_vec=f->get_num_vectors();

		mxArray* mx_result = mxCreateDoubleMatrix(1, num_vec, mxREAL);
		double* result     = mxGetPr(mx_result);
		
		CLabels* l         = NULL ;
		if (!linear)
		  l=gui->guihmm.one_class_classify();
		else
		  l=gui->guihmm.linear_one_class_classify();

		for (int i=0; i<num_vec; i++)
			result[i]=l->get_label(i);

		delete l;

		retvals[0]=mx_result;
		return true;
	}
	return false;
}

bool CGUIMatlab::one_class_hmm_classify_example(mxArray* retvals[], int idx)
{
	mxArray* mx_result=mxCreateDoubleMatrix(1, 1, mxREAL);
	double* result=mxGetPr(mx_result);
	*result=gui->guihmm.one_class_classify_example(idx);
	retvals[0]=mx_result;
	return true;
}

bool CGUIMatlab::get_svm(mxArray* retvals[])
{
	CSVM* svm=gui->guisvm.get_svm();

	if (svm)
	{
		mxArray* mx_alphas=mxCreateDoubleMatrix(svm->get_num_support_vectors(), 2, mxREAL);
		mxArray* mx_b=mxCreateDoubleMatrix(1, 1, mxREAL);

		if (mx_alphas && mx_b)
		{
			double* b=mxGetPr(mx_b);
			double* alphas=mxGetPr(mx_alphas);

			*b=svm->get_bias();

			for (int i=0; i< svm->get_num_support_vectors(); i++)
			{
				alphas[i]=svm->get_alpha(i);
				alphas[i+svm->get_num_support_vectors()]=svm->get_support_vector(i);
			}

			retvals[0]=mx_b;
			retvals[1]=mx_alphas;

			return true;
		}
	}

	return false;
}

bool CGUIMatlab::set_svm(const mxArray* vals[])
{
	CSVM* svm=gui->guisvm.get_svm();

	if (svm)
	{
		const mxArray* mx_b=vals[1];
		const mxArray* mx_alphas=vals[2];

		if (
				mx_b && mx_alphas &&
				mxGetN(mx_b) == 1 && mxGetM(mx_b) == 1 &&
				mxGetN(mx_alphas) == 2
			)
		{
			double* b=mxGetPr(mx_b);
			double* alphas=mxGetPr(mx_alphas);

			svm->create_new_model(mxGetM(mx_alphas));
			svm->set_bias(*b);

			for (int i=0; i< svm->get_num_support_vectors(); i++)
			{
				svm->set_alpha(i, alphas[i]);
				svm->set_support_vector(i, (int) alphas[i+svm->get_num_support_vectors()]);
			}

			return true;
		}
	}

	return false;
}

bool CGUIMatlab::svm_classify(mxArray* retvals[])
{
	CFeatures* f=gui->guifeatures.get_test_features();
	if (f)
	{
		int num_vec=f->get_num_vectors();
		CLabels* l=gui->guisvm.classify();

		if (!l)
		  {
		    CIO::message(M_ERROR, "svm_classify failed\n") ;
		    return false ;
		  } ;

		mxArray* mx_result=mxCreateDoubleMatrix(1, num_vec, mxREAL);
		double* result=mxGetPr(mx_result);
		for (int i=0; i<num_vec; i++)
		  result[i]=l->get_label(i);
		delete l;
		
		retvals[0]=mx_result;
		return true;
	}
	return false;
}

bool CGUIMatlab::svm_classify_example(mxArray* retvals[], int idx)
{
	mxArray* mx_result=mxCreateDoubleMatrix(1, 1, mxREAL);
	retvals[0]=mx_result;
	double* result=mxGetPr(mx_result);
	
	if (!gui->guisvm.classify_example(idx, result[0]))
	  {
	    CIO::message(M_ERROR, "svm_classify_example failed\n") ;
	    return false ;
	  } ;
	
	return true;
}

bool CGUIMatlab::set_plugin_estimate(const mxArray* vals[])
{
  int num_params = mxGetM(vals[1]) ;
  assert(mxGetN(vals[1])==2) ;
  double* result=mxGetPr(vals[1]);
  REAL* pos_params = result;
  REAL* neg_params = &(result[num_params]) ;
  double* p_size=mxGetPr(vals[2]);
  int seq_length = (int)p_size[0] ;
  int num_symbols = (int)p_size[1] ;
  assert(num_params == seq_length*num_symbols) ;

  gui->guipluginestimate.get_estimator()->set_model_params(pos_params, neg_params, seq_length, num_symbols) ;

  return true;
}

bool CGUIMatlab::get_plugin_estimate(mxArray* retvals[])
{
  REAL* pos_params, * neg_params ;
  int num_params = 0, seq_length=0, num_symbols=0 ;

  if (!gui->guipluginestimate.get_estimator()->get_model_params(pos_params, neg_params, seq_length, num_symbols))
    return false ;

  num_params = seq_length * num_symbols ;

  mxArray* mx_result=mxCreateDoubleMatrix(num_params, 2, mxREAL);
  double* result=mxGetPr(mx_result);
  for (int i=0; i<num_params; i++)
    result[i] = pos_params[i] ;
  for (int i=0; i<num_params; i++)
    result[i+num_params] = neg_params[i] ;
  
  retvals[0]=mx_result;

  mxArray* mx_size=mxCreateDoubleMatrix(1, 2, mxREAL);
  double* p_size=mxGetPr(mx_size);
  p_size[0]=(double)seq_length ;
  p_size[1]=(double)num_symbols ;
  
  retvals[1]=mx_size ;
  return true;
}

bool CGUIMatlab::plugin_estimate_classify(mxArray* retvals[])
{
	CFeatures* f=gui->guifeatures.get_test_features();
	if (f)
	{
		int num_vec=f->get_num_vectors();

		mxArray* mx_result=mxCreateDoubleMatrix(1, num_vec, mxREAL);
		double* result=mxGetPr(mx_result);
		CLabels* l=gui->guipluginestimate.classify();

		for (int i=0; i<num_vec; i++)
			result[i]=l->get_label(i);

		delete l;

		retvals[0]=mx_result;
		return true;
	}
	return false;
}

bool CGUIMatlab::plugin_estimate_classify_example(mxArray* retvals[], int idx)
{
	mxArray* mx_result=mxCreateDoubleMatrix(1, 1, mxREAL);
	double* result=mxGetPr(mx_result);
	*result=gui->guipluginestimate.classify_example(idx);
	retvals[0]=mx_result;
	return true;
}

bool CGUIMatlab::get_features(mxArray* retvals[], CFeatures* f)
{
	if (f)
	{
		///matlab can only deal with Simple (==rectangular) features
		///or sparse ones

		if (f->get_feature_class()==C_SIMPLE || f->get_feature_class()==C_SPARSE)
		{
			mxArray* mx_feat=NULL;

			switch (f->get_feature_class())
			{
				case C_SIMPLE:
					switch (f->get_feature_type())
					{
						case F_REAL:
							mx_feat=mxCreateDoubleMatrix(((CRealFeatures*) f)->get_num_vectors(), ((CRealFeatures*) f)->get_num_features(), mxREAL);

							if (mx_feat)
							{
								double* feat=mxGetPr(mx_feat);

								for (INT i=0; i<((CRealFeatures*) f)->get_num_vectors(); i++)
								{
									INT num_feat;
									bool free_vec;
									REAL* vec=((CRealFeatures*) f)->get_feature_vector(i, num_feat, free_vec);
									for (INT j=0; j<num_feat; j++)
										feat[((CRealFeatures*) f)->get_num_vectors()*j+i]= (double) vec[j];
									((CRealFeatures*) f)->free_feature_vector(vec, i, free_vec);
								}
							}
							break;
						case F_SHORT:
						case F_CHAR:
						case F_BYTE:
						default:
							CIO::message(M_ERROR, "not implemented\n");
					}
					break;
				case C_SPARSE:
					switch (f->get_feature_type())
					{
						case F_REAL:
						default:
							CIO::message(M_ERROR, "not implemented\n");
					};
					break;
				default:
					CIO::message(M_ERROR, "not implemented\n");
			}
			if (mx_feat)
				retvals[0]=mx_feat;

			return (mx_feat!=NULL);
		}
		else
			CIO::message(M_ERROR, "matlab does not support that feature type\n");

	}

	return false;
}

CFeatures* CGUIMatlab::set_features(const mxArray* vals[])
{
	const mxArray* mx_feat=vals[2];
	CFeatures* f=NULL;
	CIO::message(M_INFO, "start CGUIMatlab::set_features\n") ;

	if (mx_feat)
	{
		if (mxIsSparse(mx_feat))
		{
			CIO::message(M_ERROR, "no, no, no. this is not implemented yet\n");
		}
		else
		{
			if (mxIsDouble(mx_feat))
			{
				f= new CRealFeatures((LONG) 0);
				INT num_vec=mxGetN(mx_feat);
				INT num_feat=mxGetM(mx_feat);
				REAL* fm=new REAL[num_vec*num_feat];
				assert(fm);
				double* feat=mxGetPr(mx_feat);

				for (INT i=0; i<num_vec; i++)
				  for (INT j=0; j<num_feat; j++)
				    fm[i*num_feat+j]=feat[i*num_feat+j];
				
				((CRealFeatures*) f)->set_feature_matrix(fm, num_feat, num_vec);
			}
			else if (mxIsChar(mx_feat))
			  {
			    f= new CCharFeatures(DNA, (LONG) 0);
			    INT num_vec=mxGetN(mx_feat);
			    INT num_feat=mxGetM(mx_feat);
			    CHAR* fm=new char[num_vec*num_feat+10];
			    assert(fm);
			    mxGetString(mx_feat, fm, num_vec*num_feat+5);
			    
			    ((CCharFeatures*) f)->set_feature_matrix(fm, num_feat, num_vec);
			}
			///and so on
			else
				CIO::message(M_ERROR, "not implemented\n");
		}
	}
	return f;
}

bool CGUIMatlab::get_labels(mxArray* retvals[], CLabels* label)
{
	if (label)
	{
		mxArray* mx_lab=mxCreateDoubleMatrix(1, label->get_num_labels(), mxREAL);

		if (mx_lab)
		{
			double* lab=mxGetPr(mx_lab);

			for (int i=0; i< label->get_num_labels(); i++)
				lab[i]=label->get_label(i);

			retvals[0]=mx_lab;
			return true;
		}
	}

	return false;
}

CLabels* CGUIMatlab::set_labels(const mxArray* vals[])
{
		const mxArray* mx_lab=vals[2];

		if (mx_lab && mxGetM(mx_lab)==1 )
		{
			CLabels* label=new CLabels(mxGetN(mx_lab));

			double* lab=mxGetPr(mx_lab);

			CIO::message(M_INFO, "%d\n", label->get_num_labels());

			for (int i=0; i<label->get_num_labels(); i++)
				if (!label->set_label(i, lab[i]))
					CIO::message(M_ERROR, "weirdo ! %d %d\n", label->get_num_labels(), i);

			return label;
		}

	return NULL;
}


CHAR* CGUIMatlab::get_mxString(const mxArray* s)
{
	if ( (mxIsChar(s)) && (mxGetM(s)==1) )
	{
		int buflen = (mxGetN(s) * sizeof(mxChar)) + 1;
		CHAR* string=new char[buflen];
		mxGetString(s, string, buflen);
		return string;
	}
	else
		return NULL;
}

bool CGUIMatlab::get_kernel_matrix(mxArray* retvals[])
{
	CFeatures* f=gui->guifeatures.get_train_features();
	CFeatures* fe=gui->guifeatures.get_test_features();
	CKernel *k = gui->guikernel.get_kernel() ;
	
	if (f && fe)
	{
		int num_vece=fe->get_num_vectors();
		int num_vec=f->get_num_vectors();

		mxArray* mx_result=mxCreateDoubleMatrix(num_vec, num_vece, mxREAL);
		double* result=mxGetPr(mx_result);
		for (int i=0; i<num_vec; i++)
			for (int j=0; j<num_vece; j++)
				result[i+j*num_vec]=k->kernel(i,j) ;
		
		retvals[0]=mx_result;
		return true;
	}
	return false;
}

bool CGUIMatlab::get_kernel_tree_weights(mxArray* retvals[])
{
	CWeightedDegreeCharKernel *kernel = (CWeightedDegreeCharKernel *) gui->guikernel.get_kernel() ;
	
	if ((kernel->get_kernel_type() == K_WEIGHTEDDEGREE) && 
		(kernel->get_max_mismatch()==0))
	{
		INT len=0 ;
		REAL* res=kernel->compute_abs_weights(len) ;
		
		mxArray* mx_result=mxCreateDoubleMatrix(4, len, mxREAL);
		double* result=mxGetPr(mx_result);
		for (int i=0; i<4*len; i++)
			result[i]=res[i] ;
		delete[] res ;
		
		retvals[0]=mx_result;
		return true;
	}
	return false;
}

#endif


