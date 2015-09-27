package ca.ubc.cs.beta.models.fastrf;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.HashSet;
import java.util.TreeSet;
import java.util.Vector;

import ca.ubc.cs.beta.models.fastrf.utils.Utils;
import static ca.ubc.cs.beta.models.fastrf.utils.Hash.*;

public strictfp class Regtree implements java.io.Serializable {    
    private static final long serialVersionUID = -7861532246973394125L;
    
    public int numNodes;
    public int[] node;
    public int[] parent;
    public double[][] ysub;
    public int[] var;
    public double[] cut;
    public int[][] children;
    public int[] nodesize;
    public int npred;
    public int[][] catsplit;
    
    public double[] nodepred;
    public double[] nodevar;
    
    public boolean resultsStoredInLeaves;
    
    public boolean preprocessed;
    public double[] weightedpred;
    public double[] weightedvar;
    public double[] weights;
    
    public boolean preprocessed_for_classification;
    public double[][] bestClasses;
    
    public double[][] leafContLB;
    public double[][] leafContUB;
    public Set<Integer>[][] leafallCatValues;
    public double[][] leafDomainPercentage;
    public Vector<Integer> leafIndices;
    public boolean leafInfoIsPrecomputed = false;
    public boolean[] isCatDimension;
    public int[] categoricalDomainSizes;
    
    public int logModel;   
    
    public Regtree(int numNodes, int logModel) {
        this.numNodes = numNodes;
        this.logModel = logModel;
        preprocessed = false;
        preprocessed_for_classification = false;
    }
    
    
    public boolean equals(Object o)
    {
    	if(o instanceof Regtree)
    	{
    		Regtree rt = (Regtree) o;
    		if (npred != rt.npred) return false;
    		if (resultsStoredInLeaves != rt.resultsStoredInLeaves) return false;
    		if (preprocessed != rt.preprocessed) return false;
    		if (logModel != rt.logModel) return false;
    		
    		if (!Arrays.equals(var,rt.var)) return false;
    		if (!Arrays.equals(cut,rt.cut)) return false;
    		
    		if (!Arrays.deepEquals(catsplit,rt.catsplit)) return false;
    		if (!Arrays.deepEquals(ysub,rt.ysub)) return false;
    		if (!Arrays.deepEquals(children,rt.children)) return false;
    		
    		if (!Arrays.equals(parent,rt.parent)) return false;
    		
    		
    		
    		if (!Arrays.equals(nodesize,rt.nodesize)) return false;
    		
    		

    		if (!Arrays.equals(nodepred,rt.nodepred)) return false;
    		if (!Arrays.equals(nodevar,rt.nodevar)) return false;

    		

    		
    		if (!Arrays.equals(weightedpred,rt.weightedpred)) return false;
    		if (!Arrays.equals(weightedvar,rt.weightedvar)) return false;
    		if (!Arrays.equals(weights,rt.weights)) return false;

    		return true;

    		
    	}
    	return false;
    	
    }
    
    
    
    @SuppressWarnings("unused")
	public int hashCode()
    {
    	
    	//if (true) throw new IllegalStateException("");
    	
    	if(true) return 0;
    	System.out.println("=======");
    	
    	int hash = 0;
    	
    	hash = hash(node) + 31*hash;
    	System.out.println("HASH1:"+hash(node));
    	Arrays.toString(node);
    	
    	hash = hash(parent) + 31*hash;
    	System.out.println("HASH2:"+hash(parent));
    	Arrays.toString(parent);
    	
    	hash = hash(var) + 31*hash;
    	System.out.println("HASH3:"+hash(var));
    	
    	
    	
    	hash = hash(cut) + 31*hash;
    	System.out.println("HASH4:"+hash(cut));
    	
    	hash = hash(nodesize) + 31*hash;
    	System.out.println("HASH5:"+hash(nodesize));
    	
    	hash = npred + 31*hash;
    	System.out.println("HASH6:"+npred);
    	
    	hash = hash(children) + 31*hash;
    	System.out.println("HASH7:"+hash(children));
    	
    	hash = hash(catsplit) + 31*hash;
    	System.out.println("HASH8:"+hash(catsplit));
    	
    	hash = hash(ysub) + 31*hash;
    	System.out.println("HASH9:"+hash(ysub));
    	
    	
    	
    	hash = hash(nodepred) + 31*hash;
    	System.out.println("HASH0:"+hash(nodepred));
    	
    	hash = hash(nodevar) + 31*hash;
    	System.out.println("HASHA:"+hash(nodevar));
    	
    	hash = (resultsStoredInLeaves?1:0) + 31*hash;
    	System.out.println("HASHB:"+resultsStoredInLeaves);
    	
    	hash = preprocessed?1:0 + 31*hash;
    	System.out.println("HASHC:"+preprocessed);
    	
    	hash = hash(weightedpred) + 31*hash;
    	System.out.println("HASHD:"+hash(weightedpred));
    	
    	hash = hash(weightedvar) + 31*hash;
    	System.out.println("HASHE:"+hash(weightedvar));
    	
    	hash = hash(weights) + 31*hash;
    	System.out.println("HASHF:"+hash(weights));
    	
    	hash = logModel + 31*hash;
    	System.out.println("HASHG:"+logModel);
    	
    	return hash;
    	
    	
    }
    
    
    public Regtree(int numNodes, int ncatsplit, boolean storeResultsInLeaves, int logModel) {
        this(numNodes, logModel);
        
        resultsStoredInLeaves = storeResultsInLeaves;
        
        node = new int[numNodes];
        parent = new int[numNodes];
        var = new int[numNodes];
        cut = new double[numNodes];
        children = new int[numNodes][2];
        nodesize = new int[numNodes];
        catsplit = new int[ncatsplit][];
        
        if (resultsStoredInLeaves) {
            ysub = new double[numNodes][];
        } else {
            ysub = new double[numNodes][2]; // sum, sumofsq
        }
    }
    
    public Regtree(Regtree t) {
        this(t.numNodes, t.catsplit.length, t.resultsStoredInLeaves, t.logModel);
        
        npred = t.npred;     

        System.arraycopy(t.node, 0, node, 0, numNodes);
        System.arraycopy(t.parent, 0, parent, 0, numNodes);
        System.arraycopy(t.var, 0, var, 0, numNodes);
        System.arraycopy(t.cut, 0, cut, 0, numNodes);
        System.arraycopy(t.nodesize, 0, nodesize, 0, numNodes);
        
        for (int i=0; i < t.catsplit.length; i++) {
            catsplit[i] = new int[t.catsplit[i].length];
            catsplit[i] = new int[t.catsplit[i].length];
            System.arraycopy(t.catsplit[i], 0, catsplit[i], 0, t.catsplit[i].length);
        }
        
        for (int i=0; i < numNodes; i++) {
            children[i][0] = t.children[i][0];
            children[i][1] = t.children[i][1];
            
            if (resultsStoredInLeaves) {
                int Nnode = (var[i] == 0 ? nodesize[i] : 0);
                if (Nnode != 0) {
                    ysub[i] = new double[Nnode];
                    System.arraycopy(t.ysub[i], 0, ysub[i], 0, Nnode);
                }
            } else {
                System.arraycopy(t.ysub[i], 0, ysub[i], 0, t.ysub[i].length);
            }
        }
        
        preprocessed = t.preprocessed;
        if (preprocessed) {
            weights = new double[numNodes];
            weightedpred = new double[numNodes];
			weightedvar = new double[numNodes];
            System.arraycopy(t.weights, 0, weights, 0, numNodes);
            System.arraycopy(t.weightedpred, 0, weightedpred, 0, numNodes);
			System.arraycopy(t.weightedvar, 0, weightedvar, 0, numNodes);
        }
        
        recalculateStats();
    }
    
    /**
     * Gets a prediction for the given instantiations of features
     * @returns a matrix of size X.length*2 where index (i,0) is the prediction for X[i] 
     * and (i,1) is the variance of that prediction.
     */
    public static double[][] apply(Regtree tree, double[][] X) {
        int[] nodes = RegtreeFwd.fwd(tree, X);
        
        double[][] retn = new double[X.length][2]; // mean, var
        for (int i=0; i < X.length; i++) {
            retn[i][0] = tree.nodepred[nodes[i]];
            retn[i][1] = tree.nodevar[nodes[i]];
        }
        return retn;
    }
    
    /**
     * Classifies the given instantiations of features
     * @returns a matrix of size X.length where index i contains the 
     * most popular response for X[i]
     */
    public static double[] classify(Regtree tree, double[][] X) {
        if (!tree.preprocessed_for_classification) {
            RegtreeFwd.preprocess_for_classification(tree);
        }
        
        int[] nodes = RegtreeFwd.fwd(tree, X);
        
        double[] retn = new double[X.length];
        for (int i=0; i < X.length; i++) {
            double[] best = tree.bestClasses[nodes[i]];
            retn[i] = best[(int)(Math.random() * best.length)];
        }
        return retn;
    }
    
    /**
     * Gets a prediction for the given configurations and instances
     * @see RegtreeFwd.marginalFwd
     */
    public static Object[] applyMarginal(Regtree tree, double[][] Theta, double[][] X) {
        return RegtreeFwd.marginalFwd(tree, Theta, X);
    }
    
    public static void update(Regtree tree, double[][] newx, double[] newy) {
		if (tree.preprocessed) {
            throw new RuntimeException("Cannot update preprocessed forests.");
        }
        if (null == newx || null == newy) {
            throw new RuntimeException("Input newx or newy to update is null.");
        }
        if (newx.length != newy.length) {
            throw new RuntimeException("Argument sizes mismatch.");
        }
        if (tree.logModel > 0){
            for (int i = 0; i < newy.length; i++){
                newy[i] = Math.pow(10,newy[i]);
            }
        }
        
        int[] nodes = RegtreeFwd.fwd(tree, newx);
        boolean[] nodeChanged = new boolean[tree.node.length];
        
        for (int i = 0; i < newx.length; i++) {
            int node = nodes[i];
            nodeChanged[node] = true;

            int Nnode = tree.nodesize[node];

            if (tree.resultsStoredInLeaves) {
                double[] newysub = new double[Nnode+1];
                if (Nnode != 0) {
                    System.arraycopy(tree.ysub[node], 0, newysub, 0, Nnode);
                }
                newysub[Nnode] = newy[i];
                tree.ysub[node] = newysub;
            } else {
                tree.ysub[node][0] += newy[i]; // sum
                tree.ysub[node][1] += newy[i]*newy[i]; // sum of squares
            }

            tree.nodesize[node]++;
            while(node != 0) {
                node = tree.parent[node];
                tree.nodesize[node]++;
            }
        }
        
        for (int i=0; i < nodeChanged.length; i++) {
            if (nodeChanged[i]) {
                tree.recalculateStats(i);
            }
        }
    }

    /**
     * Recalculate statistic (mean, var) of the entire tree
     */
    public void recalculateStats() {
        nodepred = new double[numNodes];
        nodevar = new double[numNodes];
        
        for (int i=0; i < numNodes; i++) {
            recalculateStats(i);
        }
    }
    
    /**
     * Recalculate statistic (mean, var) of the specified node
     */
    public void recalculateStats(int node) {
        if (var[node] != 0) return;
        
        if (resultsStoredInLeaves) {
            nodepred[node] = Utils.mean(ysub[node]);
            nodevar[node] = Utils.var(ysub[node]);
        } else {
            double sum = ysub[node][0], sumOfSq = ysub[node][1];
            int N = nodesize[node];
            
            nodepred[node] = sum/N;
            nodevar[node] = (sumOfSq - sum*sum/N) / Math.max(N-1, 1);
        }
    }
    
    
    /**
     * Compute integral of (f-\bar{f})^2 across all values for all unobserved dimensions.
     */
    public void verifyInputsAreConsistent(int[] indicesOfObservations, double[] observations){
    	if (!leafInfoIsPrecomputed){
    		throw new RuntimeException("Leaf info has to be precomputed before predicting marginal performance.");
    	}
    	
    	//=== Verify that inputs are consistent.
    	if(indicesOfObservations.length != observations.length){
    		throw new IllegalArgumentException("indicesOfObservations and observations vectors must be of same length");
    	}
    	for (int j=0; j<observations.length; j++){
    		int index = indicesOfObservations[j];
    		if (isCatDimension[index]){
    			int value = (int) observations[j];
				if(Math.abs(observations[j] - value) > 1e-6){
					throw new IllegalArgumentException("Dimension " + indicesOfObservations[j] + " must be categorical.");
				}
				if (value < 0 || value >= categoricalDomainSizes[index]){
					throw new IllegalArgumentException("Dimension " + indicesOfObservations[j] + " is categorical with domain size " + categoricalDomainSizes[index] + ", but the input value passed was " + value + ". Note that this is 0-indexed.");
				}
    		}
    	}    	
    }
    
    /** 
     * Compute probability of data point falling into this leaf conditional on the dimensions in vector dimensions agreeing with the leaf.
     */
    private double probabilityOfLeafForRemainingDimensions(int[] dimensions, int leafIdx){
		Set<Integer> dimensionIndexSet = new HashSet<Integer>();
		for(int i=0; i<dimensions.length; i++){
			dimensionIndexSet.add(dimensions[i]); // for some reason the following failed, and so did Arrays.asList: Collections.addAll(observedIndexSet, indicesOfObservations); 
		}
		return probabilityOfLeafForRemainingDimensions(dimensionIndexSet, leafIdx);
    }

    /** 
     * Compute probability of data point falling into this leaf conditional on the dimensions in vector dimensions agreeing with the leaf.
     */
    private double probabilityOfLeafForRemainingDimensions(Set<Integer> dimensionIndexSet, int leafIdx){
		double p = 1;
		for (int j=0; j<isCatDimension.length; j++){
			if(dimensionIndexSet.contains(j)) continue;
			p = p*leafDomainPercentage[leafIdx][j];
		}
		return p;
    }
    
    /**
     * Checks whether the observations are consistent with the given leaf.
     */
    public boolean observationsAreConsistentWithLeaf(int[] indicesOfObservations, double[] observations, int leafIdx){
    	if (indicesOfObservations == null && observations == null){
    		return true;
    	}
    	
    	//=== Iterate over observations, and check one at a time, depending on whether it's continuous or categorical.
		for (int j=0; j<indicesOfObservations.length; j++){
			int o = indicesOfObservations[j];
			if (isCatDimension[o]){
				int value = (int) observations[j];
				assert(Math.abs(observations[j] - value) < 1e-6); // assert this is indeed categorical!
				
				if (!leafallCatValues[leafIdx][o].contains(value)){
					return false;
				}
			} else {
				double value = observations[j];
				if (value < leafContLB[leafIdx][o] || leafContUB[leafIdx][o] < value){
					return false;
				}
			}
		}
		return true;
    }
    
    public static Set<HashSet<Integer>> powerSet(Set<Integer> originalSet) {
        Set<HashSet<Integer>> setOfSubsets = new HashSet<HashSet<Integer>>();
        if (originalSet.isEmpty()) {
        	setOfSubsets.add(new HashSet<Integer>());
            return setOfSubsets;
        }
        List<Integer> originalList = new ArrayList<Integer>(originalSet);
        Integer head = originalList.get(0);
        Set<Integer> rest = new HashSet<Integer>(originalList.subList(1, originalList.size()));
        for (HashSet<Integer> subset: powerSet(rest)) {
            setOfSubsets.add(subset);
            HashSet<Integer> subsetPlusHead = new HashSet<Integer>();
            subsetPlusHead.add(head);
            subsetPlusHead.addAll(subset);
            setOfSubsets.add(subsetPlusHead);
        }
        return setOfSubsets;
    }
    
    /**
     * Compute \int f_S^2 dx_S. (\bar{f} is guaranteed to be zero)
     */
    public double computeFactorVariance(int[] indicesOfObservations, double[] observations, HashSet<Integer> indicesOfFactor, HashMap<Set<Integer>,Double> precomputedFactorVariance){
    	System.out.println("Computing factorVariance for " + indicesOfFactor + " ... ");
    	if (indicesOfFactor.isEmpty()) return 0;
    	if (precomputedFactorVariance.containsKey(indicesOfFactor)){
    		return precomputedFactorVariance.get(indicesOfFactor);
    	}
    	double totalVariance = computeTotalVariance(indicesOfObservations, observations, indicesOfFactor);
    	System.out.println("Total variance for " + indicesOfFactor + " is " + totalVariance);

    	Set<HashSet<Integer>> setOfSubsets = powerSet(indicesOfFactor);
    	setOfSubsets.remove(indicesOfFactor); // only look at strict subsets
    	for (HashSet<Integer> subset: setOfSubsets) {
    		double subFactorVariance = computeFactorVariance(indicesOfObservations, observations, subset, precomputedFactorVariance); 
    		totalVariance -= subFactorVariance;
	    	System.out.println("Subtracting subFactorVariance " + subFactorVariance);
		}
    	Set<Integer> copyOfIndicesOfFactorForSaving = new HashSet<Integer>();
    	copyOfIndicesOfFactorForSaving.addAll(indicesOfFactor);
    	precomputedFactorVariance.put(copyOfIndicesOfFactorForSaving, totalVariance);
    	return totalVariance;
    }
    
    
    public double computeProperTotalVariance(){
    	double totalVariance = 0;
    	int dim = isCatDimension.length;
    	for (int tmp1=0; tmp1<leafIndices.size(); tmp1++) {
    		int i = leafIndices.get(tmp1);
        	for (int tmp2=0; tmp2<leafIndices.size(); tmp2++) {
        		int j = leafIndices.get(tmp2);

				double probBothLeaves = 1;
	        	for (int d=0; d<dim; d++){
	            	if (isCatDimension[d]){
	            		Set<Integer> intersection = new HashSet<Integer>();
	            		intersection.addAll(leafallCatValues[i][d]);
	            		intersection.retainAll(leafallCatValues[j][d]);
	            		probBothLeaves *= (intersection.size()+0.0)/categoricalDomainSizes[d];
	            	} else {
	            		double lower = Math.max(leafContLB[i][d], leafContLB[j][d]);
	            		double upper = Math.min(leafContUB[i][d], leafContUB[j][d]);
	                	probBothLeaves *= (upper-lower);            		
	            	}
	            	if (probBothLeaves <= 0){ // could be < 0 if upper < lower
	            		probBothLeaves = 0;
	            		break;
	            	}
	        	}
	        	//if (probBothLeaves > 0) System.out.println("Leaves " + i + " and " + j + ": " + probBothLeaves);
				totalVariance += probBothLeaves * nodepred[i] * nodepred[j] * weights[i] * weights[j];
			}
		}
    	
    	double a_0 = 0;
    	for(Integer leafIdx:leafIndices){
    		//=== Next, compute the product of weights in this leaf for (a) the dims in the factor, and (b) the other dims.
    		double pThisFactor = probabilityOfLeafForRemainingDimensions(new TreeSet<Integer>(), leafIdx);
    		double value = weights[leafIdx] * nodepred[leafIdx];
    		a_0 += pThisFactor * value;
    	}
    	
    	return totalVariance - Math.pow(a_0,2);
    }
        
    
    /**
     * Compute \int (a_S-a_0)^2 dx_S, where S here is indicesOfFactor
     * TODO: I think this only works if indicesOfObservations and indicesOfFactor are disjoint.
     * TODO: do we divide by the right thing? Only consistent leaves?
     */
    public double computeTotalVariance(int[] indicesOfObservations, double[] observations, HashSet<Integer> indicesOfFactor){
    	if (indicesOfObservations != null && indicesOfObservations.length > 0){
        	verifyInputsAreConsistent(indicesOfObservations, observations);
    		throw new RuntimeException("observations not quite supported yet...");
    	}

		HashSet<Integer> indicesNotInFactor = new HashSet<Integer>();
		for( int i=0; i< isCatDimension.length; i++ ){
			indicesNotInFactor.add(i);
		}
		indicesNotInFactor.removeAll(indicesOfFactor);

    	double a_0 = 0;
    	for(Integer leafIdx:leafIndices){
    		if (!observationsAreConsistentWithLeaf(indicesOfObservations, observations, leafIdx)){
    			continue;
    		}		
    		//=== Next, compute the product of weights in this leaf for (a) the dims in the factor, and (b) the other dims.
    		double pThisFactor = probabilityOfLeafForRemainingDimensions(indicesNotInFactor, leafIdx);
    		double pRemainingVars=probabilityOfLeafForRemainingDimensions(indicesOfFactor, leafIdx);
    		double value = pRemainingVars * weights[leafIdx] * nodepred[leafIdx];
    		a_0 += pThisFactor * value;
    	}
//    	System.out.println("Average response a_0: " + a_0);
    	
    	double totalVariance = 0;
    	for(Integer leafIdx:leafIndices){
    		//=== Next, compute the product of weights in this leaf for (a) the dims in the factor, and (b) the other dims.
    		double pThisFactor = probabilityOfLeafForRemainingDimensions(indicesNotInFactor, leafIdx);
    		double pRemainingVars=probabilityOfLeafForRemainingDimensions(indicesOfFactor, leafIdx);
    		double value = pRemainingVars * weights[leafIdx] * nodepred[leafIdx];
        	System.out.println("Leaf " + leafIdx + ": weight=" + weights[leafIdx] + ", nodepred=" + nodepred[leafIdx] + ", value=" + value + ", a_0=" + a_0 + ", pThisFactor=" + pThisFactor + ", pRemainingVars=" + pRemainingVars);
    		totalVariance += pThisFactor*pRemainingVars * Math.pow(value-a_0,2);
    	}

    	return totalVariance;
    }
    
    
    /**
     * Compute marginal performance across unobserved dimensions.
     * 
     */
    @SuppressWarnings("unused")
	public double marginalPerformance(int[] indicesOfObservations, double[] observations){
    	verifyInputsAreConsistent(indicesOfObservations, observations);
    	
    	double result = 0;
//		for(int i=0; i<indicesOfObservations.length; i++){
//			System.out.println("Observed index set: " + indicesOfObservations[i]);
//		}
		
		double sumOfP = 0;
		double sumOfWeights = 0;
		int numConsistent = 0;
		
    	for(Integer leafIdx:leafIndices){
    		if (!observationsAreConsistentWithLeaf(indicesOfObservations, observations, leafIdx)){
    			continue;
    		}
//    		System.out.println("Consistent with leaf " + leafIdx);
    		numConsistent ++;

    		//=== Next, compute the product of weights in this leaf for the remaining dimensions.
    		double p=probabilityOfLeafForRemainingDimensions(indicesOfObservations, leafIdx);
    		
    		double pred = nodepred[leafIdx];
    		//if (logModel > 0){
    		//	result += p * Math.pow(10, pred);
    		//} else {
    			result += p * weights[leafIdx] * pred;
    		//}
    		sumOfP += p;
    		sumOfWeights += weights[leafIdx];
//    		System.out.println(" leafIdx " + leafIdx + ": weight=" + weights[leafIdx] + ", p=" + p + ", pred=" + pred);
    	}
//    	System.out.println("Total p: " + sumOfP + ", numConsistent=" + numConsistent + "/" + leafIndices.size() + ", sumOfWeights=" + sumOfWeights + ".");

//		if (logModel > 0){
//			System.out.println("logModel = " + logModel);
//			result = Math.log10(result);
//		}

    	return result;
    }
    
    /**
     * Precompute which ranges of parameters fall into which leaves.
     * isCat holds a Boolean for each of dim dimensions, true for categorical and false for continuous parameters
     * allCatValues is dim-dimensional, holding empty sets for continuous dimensions and values 0,...,k-1 for categorical dimensions of domainSize k 
     * contLB and contUB are dim-dimensional, holding 0 for categorical dimensions
     */
    
	@SuppressWarnings("unchecked")
	public void precomputeLeafInfo(boolean[] isCat, HashSet<Integer>[] allCatValues, double[] contLB, double[] contUB){
    	isCatDimension = isCat;
    	int dim = isCat.length;
    	leafIndices = new Vector<Integer>();
    	categoricalDomainSizes = new int[dim];
    	for (int i=0; i<dim; i++){
    		if( allCatValues[i] == null ){
    			categoricalDomainSizes[i] = 0;
    		} else {
    			categoricalDomainSizes[i] = allCatValues[i].size();
    		}
    	}
        
    	//=== Initialize the results of this precomputation.
        leafContLB = new double[numNodes][dim];
        leafContUB = new double[numNodes][dim];
        leafallCatValues = new Set[numNodes][dim];
        leafDomainPercentage = new double[numNodes][dim];
        for (int i = 0; i < numNodes; i++) {
        	leafContLB[i] = new double[dim];
        	leafContUB[i] = new double[dim];
        	leafallCatValues[i] = new Set[dim];
        	leafDomainPercentage[i] = new double[dim];
		}
        
        precomputeLeafInfoInSubtree(0, isCat, allCatValues, contLB, contUB);
        leafInfoIsPrecomputed = true;
    }
    
    /**
     * Recursive method doing the work for precomputeLeafInfo.
     */
    private void precomputeLeafInfoInSubtree(int thisnode, boolean[] isCat, HashSet<Integer>[] allCatValues, double[] contLB, double[] contUB){
//    	System.out.println("Preprocessing node " + thisnode);
    	int splitvar = var[thisnode];
    	
    	if (splitvar == 0) {
    		//=== Node thisnode is a leaf -> save statistics.
        	leafIndices.add(thisnode);
        	int dim = isCat.length;
        	leafallCatValues[thisnode] = allCatValues;
        	for (int i=0; i<dim; i++){
            	leafContLB[thisnode][i] = contLB[i];
            	leafContUB[thisnode][i] = contUB[i];
//            	leafallCatValues[thisnode][i] = (Set<Integer>) allCatValues[i].clone();
            	if (isCat[i]){
            		leafDomainPercentage[thisnode][i] = (allCatValues[i].size()+0.0) / categoricalDomainSizes[i]; 
            	} else {
                	leafDomainPercentage[thisnode][i] = contUB[i]-contLB[i];            		
            	}
        	}
        	
//        	System.out.println("Saving leafDomainPercentage in leaf " + thisnode + " for first param: " + leafDomainPercentage[thisnode][0]);
    	} else {
    		//=== Double-check that we have categorical variables right.
    		if (splitvar>0){
    			assert(!isCat[splitvar]);
    		} else {
    			assert(isCat[-splitvar]);
    		}
    		
            int left_kid = children[thisnode][0];
            int right_kid = children[thisnode][1];
            
            if (Math.abs(splitvar) > isCat.length){
            	//=== This is a split on an instance feature -> we recurse left and right with the same inputs.
                precomputeLeafInfoInSubtree(left_kid,  isCat, allCatValues, contLB, contUB);
                precomputeLeafInfoInSubtree(right_kid, isCat, allCatValues, contLB, contUB);
            	
            } else {
            	//=== This is a split on a parameter value -> we split the domain of that parameter.
            	
	            double cutoff = cut[thisnode];
	            if (splitvar > 0) { 
	                //=== We're splitting on a continuous variable.
	                int v = splitvar-1;
	                
	                //=== Recurse to left child.
	                double previousUB = contUB[splitvar-1]; // save previous upper bound
	                contUB[v] = cutoff;
	                precomputeLeafInfoInSubtree(left_kid, isCat, allCatValues, contLB, contUB);
	                contUB[v] = previousUB;                 // restore previous upper bound
	                
	                //=== Recurse to right child.
	                double previousLB = contLB[splitvar-1];
	                contLB[v] = cutoff;
	                precomputeLeafInfoInSubtree(right_kid, isCat, allCatValues, contLB, contUB);
	                contLB[v] = previousLB;
	                
	            } else { 
	            	//=== We're splitting on a categorical variable.
	            	int v = -splitvar-1;
	            	HashSet<Integer> thisValues = allCatValues[v];
	            	HashSet<Integer> leftValues = new HashSet<Integer>();
	            	HashSet<Integer> rightValues = new HashSet<Integer>();
	            	
	            	//=== Split values into left and right kid.
	            	for( Integer thisValue:thisValues ){
	            		int split = catsplit[(int)cutoff][thisValue]; 
	            		if(split==0){
	            			leftValues.add(thisValue);
	            		} else if (split==1){
	            			rightValues.add(thisValue);
	            		} else {
	            			throw new IllegalStateException("Error in node " + thisnode + ": catsplit does not state which kid to propagate value " + thisValue + " to. Note that input allCatValues should be 0-indexed!");
	            		}
	            	}
//            		System.out.println("Left values: " + leftValues);
//            		System.out.println("Right values: " + rightValues);
	            	
//            		System.out.println("Left recursion for parameter " + v + " (0-indexed) with values " + leftValues);
	            	//=== Recurse to left kid.
	            	HashSet<Integer>[] allLeftValues = allCatValues.clone(); 
	            	allLeftValues[v] = leftValues;
	                precomputeLeafInfoInSubtree(left_kid, isCat, allLeftValues, contLB, contUB);
	
//            		System.out.println("Right recursion for parameter " + v + " (0-indexed) with values " + rightValues);
	            	//=== Recurse to right kid.
	                HashSet<Integer>[] allRightValues = allCatValues.clone(); 
	            	allRightValues[v] = rightValues;
	                precomputeLeafInfoInSubtree(right_kid, isCat, allRightValues, contLB, contUB);
	            }
            }
    	}
    }
}
