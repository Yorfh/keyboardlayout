package ca.ubc.cs.beta.models.fastrf.utils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Vector;

public class RfData implements java.io.Serializable {
	private static final long serialVersionUID = 239457234672435L;
	public RfData(ArrayList<TripletDoubleArrayDoubleArrayDouble> data, int[] catDomainSizes) {
		int numTheta = data.get(0).theta.length;
		int numX = data.get(0).x.length;
		
		Theta = new double[data.size()][numTheta];
		X = new double[data.size()][numX];
		y = new double[data.size()];

		for (int i=0; i<data.size(); i++) {
			TripletDoubleArrayDoubleArrayDouble triplet = data.get(i);
			System.arraycopy(triplet.theta, 0, Theta[i], 0, numTheta);
			System.arraycopy(triplet.x, 0, X[i], 0, numX);
			y[i] = triplet.y;
		}
		
		construct(Theta, X, y, catDomainSizes);
	}

	public RfData(double[][] Theta_nonuniq, double[][] X_nonuniq, double[] y, int[] catDomainSizes) {
		construct(Theta_nonuniq, X_nonuniq, y, catDomainSizes);
	}
	
	private void construct(double[][] Theta_nonuniq, double[][] X_nonuniq, double[] y, int[] catDomainSizes) {
		Theta = Theta_nonuniq;
		X = X_nonuniq;
		this.y = y;
		this.catDomainSizes = catDomainSizes;
		this.theta_inst_idxs = new int[Theta.length][2];
		for(int i=0; i<theta_inst_idxs.length; i++){
			theta_inst_idxs[i][0] = i;
			theta_inst_idxs[i][1] = i;
		}
		makeUnique(true);
		makeUnique(false);
	}

	private double[][] Theta;
	private double[][] X;
	private double[] y;
	private int[][] theta_inst_idxs;
	private int[] catDomainSizes;
	
	public double[][] getTheta() {
		return Theta;
	}
	public double[][] getX() {
		return X;
	}
	public double[] getY() {
		return y;
	}
	public int[][] getTheta_inst_idxs() {
		return theta_inst_idxs;
	}
	public int[] getCatDomainSizes() {
		return catDomainSizes;
	}
	
	/* 
	 * Make Theta unique, and updates the first column of theta_inst_idxs accordingly.  
	 */
	public void makeUnique(boolean makeThetaUnique){
		Set<ArrayWithSensibleEquals> uniqueArrays = new HashSet<ArrayWithSensibleEquals>();
		Map<ArrayWithSensibleEquals,Integer> mapArrayToUniqueIndex = new HashMap<ArrayWithSensibleEquals, Integer>();
		Vector<Integer> newIndices = new Vector<Integer>();
		
		//=== Collect unique theta arrays in a set,	building up a vector mapping from previous to unique indices as we go.
		if (makeThetaUnique){
			for (int i = 0; i < Theta.length; i++) {
				double[] theta = Theta[i];
				ArrayWithSensibleEquals array = new ArrayWithSensibleEquals(theta);
				if(!uniqueArrays.contains(array)){
					mapArrayToUniqueIndex.put( array, uniqueArrays.size() );
					uniqueArrays.add( array );
				}
				newIndices.add(i, mapArrayToUniqueIndex.get(array) );
			}
		} else {
			for (int i = 0; i < X.length; i++) {
				double[] x = X[i];
				ArrayWithSensibleEquals array = new ArrayWithSensibleEquals(x);
				if(!uniqueArrays.contains(array)){
					mapArrayToUniqueIndex.put( array, uniqueArrays.size() );
					uniqueArrays.add( array );
				}
				newIndices.add(i, mapArrayToUniqueIndex.get(array) );
			}
		}
		
		//=== Create the unique array, fill it, and then change the reference to the new unique array. Also update theta_inst_idxs.
		if( makeThetaUnique ){
			double[][] uniqueTheta = new double[uniqueArrays.size()][];
			for (ArrayWithSensibleEquals array: uniqueArrays) {
				uniqueTheta[mapArrayToUniqueIndex.get(array)] = array.values;
			}
			Theta = uniqueTheta;
			for (int i = 0; i < newIndices.size(); i++) {
				theta_inst_idxs[i][0] = newIndices.get(i);
			}
		} else {
			double[][] uniqueX = new double[uniqueArrays.size()][];
			for (ArrayWithSensibleEquals array: uniqueArrays) {
				uniqueX[mapArrayToUniqueIndex.get(array)] = array.values;
			}
			X = uniqueX;
			for (int i = 0; i < newIndices.size(); i++) {
				theta_inst_idxs[i][1] = newIndices.get(i);
			}			
		}
	}
	
	/*
	 * Builds a matrix combining Theta and X that we can directly feed to apply.
	 * Dimensions: Let Theta be M x D, X be K x d, and theta_inst_idxs be N x 2; then, the combined matrix is N x (D+d).    
	 */
	public double[][] buildMatrixForApply(){
		assert(Theta.length > 0);
		assert(X.length > 0);
		int numPoints = theta_inst_idxs.length;
		int dimTheta = Theta[0].length;
		int dimX = X[0].length;
		double[][] combinedMatrix = new double[numPoints][dimTheta + dimX];

		for (int i = 0; i < numPoints; i++) {
			for (int j = 0; j < dimTheta; j++) {
				combinedMatrix[i][j] = Theta[theta_inst_idxs[i][0]][j];		
			}
			for (int j = dimTheta; j < dimTheta+dimX; j++) {
				combinedMatrix[i][j] = X[theta_inst_idxs[i][1]][j-dimTheta];		
			}
		}
		return combinedMatrix;
	}
	
}


class ArrayWithSensibleEquals{
	public ArrayWithSensibleEquals(double[] values) {
		this.values = values;
	}
	public double[] values;
    public boolean equals(Object o){
    	return (o==null? false : Arrays.equals(((ArrayWithSensibleEquals)o).values, values));
    }
    public int hashCode() {
        return Arrays.hashCode(values);
    }
}