package ca.ubc.cs.beta.models.fastrf;

import java.util.HashMap;
import java.util.Map;
import java.util.Random;
import java.util.Arrays;

/**
 * @param catDomainSizes a vector of size X[0].length indicating the size of the domain for the corresponding categorical feature, or 0 if the feature is continuous.
 *                       catDomainSizes should be the length of the number of Parameters + Number of Features.
 * @param condParents the conditional parents of the given variable (column index into X). Empty if no parents. NOT SORTED
 * @param condParentVals the OK values of all the conditional parents of the given variable (each element is a matrix). Indexed wrt condParents.
 * @param splitMin the minimum number of data points in each node.
 * @param ratioFeatures the percentage (0, 1] of the features to use in deciding the split at each node.
 * @param logModel whether to build a model in log space
 * @param storeResponses whether to store the responses themselves in the leaves or just to store some statistic (sum, sumofsquares, and leaf size)
 * @param seed -1 means don't use a seed (i.e. create a new Random but don't call setSeed). 
 * @param random will be used instead of seed if it's not null.
 * @param minVariance - Minimum Variance value that will ever be returned on apply call
 */
public strictfp class RegtreeBuildParams implements java.io.Serializable {    
	public RegtreeBuildParams(int numVars, boolean doBootstrapping) {
		this(numVars, doBootstrapping, 10);
	}
	
	public RegtreeBuildParams(int numVars, boolean doBootstrapping, int splitMin) {
		this(numVars, doBootstrapping, splitMin, 2.0/3);
	}
	
	public RegtreeBuildParams(boolean doBootstrapping, int splitMin, int[] catDomainSizes) {
		this( doBootstrapping, splitMin, 2.0/3, catDomainSizes);
	}
	
	public RegtreeBuildParams(int numVars, boolean doBootstrapping, int splitMin, double ratioFeatures) {
		this( doBootstrapping, splitMin, ratioFeatures, new int[numVars]); //initializes catDomainSizes to all zero
	}
	
	public RegtreeBuildParams(boolean doBootstrapping, int splitMin, double ratioFeatures, int[] catDomainSizes) {
		super();
		this.catDomainSizes = catDomainSizes; 
		this.doBootstrapping = doBootstrapping;
		this.splitMin = splitMin;
		this.ratioFeatures = ratioFeatures;
		this.random = new Random(3);
	}
	

	private static final long serialVersionUID = -6803645785543626390L;
	public int[] catDomainSizes;
    
	//DEPRECATED
	//public int[][] condParents = null;
    //public int[][][] condParentVals = null;
    
    /*
     * mapping from variable index to disjunctions of condition clauses (i.e., conjunctions) with parent variable index
     */
    public Map<Integer, int[][]> nameConditionsMapParentsArray;
    /*
     * mapping from variable index to disjunctions of condition clauses (i.e., conjunctions) with parent variable value*s* (based on conditional operator, only one value (>,<,!=,==) or several values ("in") 
     */
    public Map<Integer, double[][][]> nameConditionsMapParentsValues;
    /*
     * mapping from variable index to dijunctions of condition clauses (i.e., conjunctions) with conditional operators (<,>,!=,==,"in") 
     */
    public Map<Integer, int[][]> nameConditionsMapOp;
    
    public double minVariance;
    public String toString()
    {
        try{
        StringBuffer sb = new StringBuffer();
        sb.append("catDomain: (" + ((catDomainSizes != null) ? catDomainSizes.length : "null") +")");
        sb.append(Arrays.toString(catDomainSizes));
        sb.append("\n");
        sb.append("\n");
        //sb.append("condParent: (" + ((condParents != null) ? condParents.length : "null") +")");
        //sb.append(Arrays.deepToString(condParents));
        sb.append("\n");
        //sb.append("condParentVals: (" + ((condParentVals != null) ? condParentVals.length : "null") + ")");
        //sb.append(Arrays.deepToString(condParentVals));
        sb.append("\nSplitMin:" + splitMin);
        sb.append("\nRatioFeatures:" + ratioFeatures);
        //sb.append("\nCutOffPenaltyFactor:" + cutoffPenaltyFactor);
        sb.append("\nLogModel:" + logModel);
        sb.append("\nStoreResponses:" + storeResponses);
       
        return sb.toString();
        } catch(RuntimeException e)
        {
            e.printStackTrace();
            return "RuntimeException occured building RegtreeBuildPalams";
        }
        
        
    }
    
    public int splitMin;
    public void setSplitMin(int splitMin) {
		this.splitMin = splitMin;
	}

    public boolean doBootstrapping = true;

	public double ratioFeatures;
    public int logModel;
    public boolean storeResponses;
    
    public long seed = -1;
    public Random random = null;
	public boolean brokenVarianceCalculation = true;
    
    /**
     * DEPRECATED
     * Matlab cell arrays don't carry over to Java very well so create the conditional arrays from the cell arrays
     * Called from matlab instead of setting the condParents and condParentVals arrays
     */
   /* public void conditionalsFromMatlab(int[] cond, int[] condParent, Object[] condParentValsObj, int nvars) {
        condParents = new int[nvars][];
        condParentVals = new int[nvars][][];
        
        for (int i=0; i < nvars; i++) {
            int count = 0;
            for (int j=0; j < cond.length; j++) {
                if (cond[j]-1 == i) {
                    count++;
                }
            }
            condParents[i] = new int[count];
            condParentVals[i] = new int[count][];
            
            count = 0;
            for (int j=0; j < cond.length; j++) {
                if (cond[j]-1 == i) {
                    condParents[i][count] = condParent[j]-1;
                    if(condParentValsObj[j] instanceof int[]) {
                        condParentVals[i][count] = (int[])condParentValsObj[j];
                    } else {
                        condParentVals[i][count] = new int[1];
                        condParentVals[i][count][0] = (Integer)condParentValsObj[j];
                    }
                    count++;
                }
            }
        }
    }
	*/

	public void setLogModel(int logModel) {
		this.logModel = logModel;
	}

	public static RegtreeBuildParams copy(RegtreeBuildParams bp, int size)
	{
			RegtreeBuildParams bpNew = new RegtreeBuildParams(size, bp.doBootstrapping);
			
			bpNew.logModel = bp.logModel;
			bpNew.brokenVarianceCalculation = bp.brokenVarianceCalculation;
			
			bpNew.catDomainSizes = new int[size];
			//bpNew.condParents = new int[size][];
			//bpNew.condParentVals = new int[size][][];
			System.arraycopy(bp.catDomainSizes, 0,bpNew.catDomainSizes, 0, size);
			//System.arraycopy(bp.condParents, 0,bpNew.condParents, 0, size);
			//System.arraycopy(bp.condParentVals, 0,bpNew.condParentVals, 0, size);
			bpNew.nameConditionsMapParentsArray = new HashMap<Integer, int[][]>(bp.nameConditionsMapParentsArray);
			bpNew.nameConditionsMapParentsValues = new HashMap<Integer, double[][][]>(bp.nameConditionsMapParentsValues);
			bpNew.nameConditionsMapOp = new HashMap<Integer, int[][]>(bp.nameConditionsMapOp);
			
			bpNew.random = bp.random;
			bpNew.ratioFeatures = bp.ratioFeatures;
			bpNew.minVariance = bp.minVariance;
			bpNew.splitMin = bp.splitMin;
			bpNew.storeResponses = bp.storeResponses;
			
			bpNew.seed = bp.seed;
			return bpNew;
	}
	
}
