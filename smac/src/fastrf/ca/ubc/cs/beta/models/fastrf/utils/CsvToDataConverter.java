package ca.ubc.cs.beta.models.fastrf.utils;

import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Vector;

import com.opencsv.CSVReader;

public class CsvToDataConverter implements java.io.Serializable {
	private static final long serialVersionUID = 398638943689467378L;
	private HashMap<Integer, Integer> csvColToModelDimMap;
	private HashMap<Integer, Map<String,Integer>> catDomainValueMaps;
	private int[] catDomainSizes;
	private String[] csvHeader;
	private int[] thetaColIdxs;
	private int[] xColIdxs;
	private int yColIdx;
	private int[] catColIdxs;
	private String[] parameterNames;
	private String[] featureNames;

	public HashMap<Integer, Map<String,Integer>> getCatDomainValueMaps(){
		return catDomainValueMaps;
	}
	public int[] getCatDomainSizes() {
		return catDomainSizes;
	}

	public String[] getParameterNames() {
		return parameterNames;
	}

	public String[] getFeatureNames() {
		return featureNames;
	}

	public CsvToDataConverter(String filename) throws IOException{
		getColIdxsFromCsvHeader(filename);
		prepareConverter(filename, thetaColIdxs, xColIdxs, yColIdx, catColIdxs);
	}
	
	public CsvToDataConverter(String filename, int[] thetaColIdxs, int[] xColIdxs, int yColIdx, int[] catColIdxs) throws IOException{
		prepareConverter(filename, thetaColIdxs, xColIdxs, yColIdx, catColIdxs);
	}
	
	private void getColIdxsFromCsvHeader(String csvFilename) throws IOException{
		//== Read header.
		CSVReader csvReader = new CSVReader(new FileReader(csvFilename));
		csvHeader = csvReader.readNext();
		csvReader.close();
		
		//== Get values for thetaColIdx, xColIdx, and yColIdx from header.
		Vector<Integer> paramIndices = new Vector<Integer>();
		Vector<Integer> featureIndices = new Vector<Integer>();
		Vector<Integer> categoricalIndices = new Vector<Integer>();
		int responseIndex = -1;
		for (int i = 0; i < csvHeader.length; i++) {
			if( csvHeader[i].trim().toLowerCase().startsWith("parameter")){
				paramIndices.add(i);
			}
			if( csvHeader[i].trim().toLowerCase().startsWith("feature")){
				featureIndices.add(i);
			}
			if( csvHeader[i].toLowerCase().contains("categorical")){
				categoricalIndices.add(i);
			}
			if( csvHeader[i].trim().toLowerCase().startsWith("performance")){
				if (responseIndex != -1){
					System.out.println("ERROR. Only one column in the input CSV is allowed to start with the string performance, but there are multiple ones. Please fix this; I'm exiting.");
					System.exit(-1);
				}
				responseIndex = i;
			}
		}
		if (responseIndex == -1){
			System.out.println( "ERROR. In the input CSV, the header for the column with the response to be optimized must start with the string performance (that's how it is detected). Please fix this; I'm exiting." );
			System.exit(-1);						
		}
		if( paramIndices.isEmpty() ){
			System.out.println("ERROR. At least one column in the input CSV has to hold a parameter (the header for parameter columns starts with the string Parameter). Please fix this; I'm exiting.");
			System.exit(-1);			
		}
		if( featureIndices.isEmpty() ){
			System.out.println("ERROR. At least one column in the input CSV has to hold a feature (the header for feature columns starts with the string Feature). Please fix this; I'm exiting.");
			System.exit(-1);			
		}
		
		//== Construct thetaColIdx, xColIdx, and yColIdx.  
		thetaColIdxs = new int[paramIndices.size()];
		for (int i = 0; i < thetaColIdxs.length; i++) {
			thetaColIdxs[i] = paramIndices.get(i);
		}
		xColIdxs = new int[featureIndices.size()];
		for (int i = 0; i < xColIdxs.length; i++) {
			xColIdxs[i] = featureIndices.get(i);
		}
		catColIdxs = new int[categoricalIndices.size()];
		for (int i = 0; i < catColIdxs.length; i++) {
			catColIdxs[i] = categoricalIndices.get(i);
		}
		yColIdx = responseIndex;
	}
	
	private List<String[]> readEntriesFromCsv(String filename, boolean overWriteHeader) throws IOException{
		CSVReader csvReader = new CSVReader(new FileReader(filename));
		List<String[]> entries = csvReader.readAll();
		csvReader.close();

		if (overWriteHeader){
			csvHeader = entries.get(0);
		} else {
			for (int i = 0; i < csvHeader.length; i++) {
				assert(csvHeader[i].equals(entries.get(0)[i]));
			}
		}
		entries.remove(0);
		return entries;
	}

	//=== Since the csv file columns are not ordered as all theta first, then all x, we need a map of indices: csvColToModelDimMap[csv index] -> model index   
	private void computeCsvToModelIdxMap(int[] thetaColIdxs, int[] xColIdxs){
		csvColToModelDimMap = readOutCsvToModelIdxMap(thetaColIdxs, xColIdxs);
	}
	
	public static HashMap<Integer, Integer> readOutCsvToModelIdxMap(int[] thetaColIdxs, int[] xColIdxs){
		HashMap<Integer, Integer> localCsvColToModelDimMap = new HashMap<Integer, Integer>();
		for (int i = 0; i < thetaColIdxs.length; i++) {
			localCsvColToModelDimMap.put(thetaColIdxs[i], i);
		}
		for (int i = 0; i < xColIdxs.length; i++) {
			localCsvColToModelDimMap.put(xColIdxs[i], thetaColIdxs.length + i);
		}
		return localCsvColToModelDimMap;
	}
	
	private void computeCatDomainValueMap(List<String[]> lines, int[] catColIdxs, Map<Integer, Integer> csvColToModelDimMap){
		catDomainValueMaps = readOutCatDomainValueMap(lines, catColIdxs, csvColToModelDimMap);
	}
	
	//=== For all categorical variables, build a map from their values to an index: catDomainValueMaps[inputDim]: { domainValue0 -> 0, ..., domainValueN -> N  }
	public static HashMap<Integer, Map<String,Integer>> readOutCatDomainValueMap(List<String[]> lines, int[] catColIdxs, Map<Integer, Integer> csvColToModelDimMap){
		HashMap<Integer, Map<String,Integer>> localCatDomainValueMaps = new HashMap<Integer, Map<String,Integer>>();	
		
		for (int i = 0; i < catColIdxs.length; i++) {
			int csvColIdx = catColIdxs[i];
			int modelColIdx = csvColToModelDimMap.get(csvColIdx);
			Map<String, Integer> domainValueMap = new HashMap<String, Integer>();
			for (int j = 0; j < lines.size(); j++) {
				String valueString = lines.get(j)[csvColIdx].trim();
				if( domainValueMap.get(valueString) == null ){
					domainValueMap.put(valueString, domainValueMap.size()+1);
				}
			}
			localCatDomainValueMaps.put(modelColIdx, domainValueMap);
		}
		return localCatDomainValueMaps;
	}
	
	//=== Set catDomainSizes accordingly to the values maps.
	private void computeCatDomainSizes(HashMap<Integer, Map<String,Integer>> catDomainValueMaps, int numDim){
		catDomainSizes = readOutCatDomainSizes(catDomainValueMaps, numDim);
	}
		
	public static  int[] readOutCatDomainSizes(HashMap<Integer, Map<String,Integer>> catDomainValueMaps, int numDim){	
		int[] localCatDomainSizes = new int[numDim];
		for (int i = 0; i < numDim; i++) {
			localCatDomainSizes[i] = 0;
			if (catDomainValueMaps.get(i) != null){
				localCatDomainSizes[i] = catDomainValueMaps.get(i).size();
			}
		}
		return localCatDomainSizes;
	}
	
	private void prepareConverter(String filename, int[] thetaColIdxs, int[] xColIdxs, int yColIdx, int[] catColIdxs) throws IOException{
		this.thetaColIdxs = thetaColIdxs;
		this.xColIdxs = xColIdxs;
		this.yColIdx = yColIdx;
		this.catColIdxs = catColIdxs;
		
		//=== Use helper functions to get indices right and get categorical variables prepared.
		computeCsvToModelIdxMap(thetaColIdxs, xColIdxs);
		List<String[]> lines = readEntriesFromCsv(filename, true);
		computeCatDomainValueMap(lines, catColIdxs, csvColToModelDimMap);
		computeCatDomainSizes(catDomainValueMaps, thetaColIdxs.length + xColIdxs.length);
		
		parameterNames = new String[thetaColIdxs.length];
		for (int i = 0; i < thetaColIdxs.length; i++) {
			parameterNames[i] = csvHeader[thetaColIdxs[i]].trim();
		}

		featureNames = new String[xColIdxs.length];
		for (int i = 0; i < xColIdxs.length; i++) {
			featureNames[i] = csvHeader[xColIdxs[i]].trim();
		}
}
	
	/*
	 *  Get the int value for a categorical value string for parameter j.
	 */
	public int getIntForCategoricalValueString(int j, String categoricalValueString){
		if (catDomainSizes[j] == 0){ // not categorical
			throw new RuntimeException("Parameter " + j + " is not categorical, so its categorical index is not defined.");	
		} else {
			int thetaValue = catDomainValueMaps.get(j).get(categoricalValueString);
			return thetaValue;
		}
	}
	
	/*
	 *  Read the data from a csv file (asserting that the file is of the same form as the one used to construct the converter).
	 */
	public RfData readDataFromCsvFile(String filename) throws IOException{
		List<String[]> lines = readEntriesFromCsv(filename, false);
		
		//=== Initialize Theta, X, y, and theta_inst_idxs. 
		double[][] Theta_nonuniq = new double[lines.size()][thetaColIdxs.length];
		double[][] X_nonuniq = new double[lines.size()][xColIdxs.length];
		double[] y = new double[lines.size()];
		
		//=== Load data into Theta, X, y, and theta_inst_idxs, one line at a time. 
		for (int i = 0; i < y.length; i++) {
			String[] line = lines.get(i);
			for (int j = 0; j < thetaColIdxs.length; j++) {
				int csvColIdx = thetaColIdxs[j];
				int modelColIdx = csvColToModelDimMap.get(csvColIdx);
				
				String valueString = line[csvColIdx].trim();
				if (catDomainSizes[modelColIdx] == 0){ // not categorical
					Theta_nonuniq[i][modelColIdx] = Double.parseDouble(valueString);			
				} else {
					int thetaValue = getIntForCategoricalValueString(modelColIdx, valueString);
//					int thetaValue = catDomainValueMaps.get(modelColIdx).get(valueString); // equivalent, but let's use the same method for this everywhere.
					Theta_nonuniq[i][modelColIdx] = thetaValue;					
				}					
			}
			for (int j = 0; j < xColIdxs.length; j++) {
				int csvColIdx = xColIdxs[j];
				int modelColIdx = csvColToModelDimMap.get(csvColIdx);
				String valueString = line[csvColIdx].trim();
				if (catDomainSizes[modelColIdx] == 0){ // not categorical
					X_nonuniq[i][modelColIdx-thetaColIdxs.length] = Double.parseDouble(valueString);
				} else {
					if ( catDomainValueMaps.get(modelColIdx).containsKey(valueString) ){
						int xValue = catDomainValueMaps.get(modelColIdx).get(valueString);
						X_nonuniq[i][modelColIdx-thetaColIdxs.length] = xValue;											
					} else {
						throw new IllegalArgumentException("Error in reading csv file: value for column " + csvColIdx + " (" + csvHeader[csvColIdx] + ") is " + valueString + "; the domain only has the values " + catDomainValueMaps.get(modelColIdx).keySet());
//						System.out.println("\n\n======================================================\nError in reading csv file: value for column " + csvColIdx + " (" + csvHeader[csvColIdx] + ") is " + valueString + "; the domain only has the values " + catDomainValueMaps.get(modelColIdx).keySet() + "\n\n REPLACING WITH INDEX 0 \n\n======================================================\n");
//						data.X[i][modelColIdx-thetaColIdxs.length] = 1;
					}
				}					
			}
			y[i] = Double.parseDouble(line[yColIdx]);
		}
		
		return new RfData(Theta_nonuniq, X_nonuniq, y, catDomainSizes);
	}
	
	public String getPcsString(String filename) throws IOException{
		List<String[]> lines = readEntriesFromCsv(filename, false);
		String pcsString = "";
		//=== Construct pcs file string (with meaningless defaults).
		for (int i = 0; i < thetaColIdxs.length; i++) {
			if( catDomainSizes[i] == 0 ){
				//== Determine min and max of continuous parameter and append the parameter.
				double minValue = Double.POSITIVE_INFINITY;
				double maxValue = Double.NEGATIVE_INFINITY;
				for (int j = 1; j < lines.size(); j++) {
					double value = Double.parseDouble( lines.get(j-1)[thetaColIdxs[i]].trim() );
					minValue = Math.min(minValue, value);
					maxValue = Math.max(maxValue, value);
				}
				pcsString = pcsString + csvHeader[thetaColIdxs[i]] + "[" + minValue + "," + maxValue + "] [" + (minValue+maxValue)/2 +"]\n";
			} else {
				//== Get values for parameter in the order we indexed them in.
				Set<String> valueSet = catDomainValueMaps.get(i).keySet();
				Vector<String> valuesSorted = new Vector<String>(valueSet.size());
				for (String valueString : valueSet) {
					valuesSorted.add(catDomainValueMaps.get(i).get(valueString), valueString);
				}
				//== Add parameter description to the pcs string.
				pcsString = pcsString + csvHeader[thetaColIdxs[i]] + "{";
				for (int j = 0; j < valuesSorted.size()-1; j++) {
					pcsString = pcsString + valuesSorted.get(j) + ",";
				}
				pcsString = pcsString + valuesSorted.get(valuesSorted.size()-1) + "} [" + valuesSorted.get(0) + "]\n"; 
			}
		}
		return pcsString;
	}
	
	/*
	 * Read feature values from the columns with matching header from a new CSV file.
	 */
	public double[][] readXFromOtherCsvFile(String testFileCsvName) throws IOException{
		//== Read header and collect matching X indices.
		CSVReader csvReader = new CSVReader(new FileReader(testFileCsvName));
		String[] testHeader = csvReader.readNext();
		csvReader.close();
		
		int[] testXColIdxs = new int[xColIdxs.length];
		for (int i = 0; i < xColIdxs.length; i++) {
			testXColIdxs[i] = -1;
			for (int j = 0; j < testHeader.length; j++) {
				if(testHeader[j].equals(csvHeader[xColIdxs[i]])){
					testXColIdxs[i] = j;
				}
			}
			if(testXColIdxs[i] == -1){
				System.out.println("Error: test CSV does not contain one of the features of the training file: " + csvHeader[xColIdxs[i]] + ". Please fix this; I'm exiting.");
				System.exit(-1);
			}
		}
		
		//=== Read in data from test file.
		csvReader = new CSVReader(new FileReader(testFileCsvName));
		List<String[]> testLines = csvReader.readAll();
		csvReader.close();
		
		double[][] X = new double[testLines.size()-1][testXColIdxs.length];
		
		for (int i = 0; i < X.length; i++) {
			for (int j = 0; j < testXColIdxs.length; j++) {
				double value = Double.parseDouble( testLines.get(i+1)[testXColIdxs[j]].trim() );
				X[i][j] = value;
			}			
		}
		return X;
	}
	
}
