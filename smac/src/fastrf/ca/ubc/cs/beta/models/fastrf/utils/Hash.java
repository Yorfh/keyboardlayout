package ca.ubc.cs.beta.models.fastrf.utils;
/**
 * Utility class to handle generating lossy float based hashes on objects
 * (If not in strictfp mode, the last few digits on 
 * @author seramage
 *
 */
public strictfp class Hash {
	
	public static int hash(int[] obj)
	{
		return hashCode(obj);
	}

	public static int hash(int[][] obj)
	{
		return hashCode(obj);
	}
	
	protected static float[] cFloat(double[] value)
	{
		if(value == null) return null;
		float[] arr = new float[value.length];
		for(int i=0; i < value.length; i++)
		{
			arr[i] = (float) value[i];
		}
		return arr;
	}
	
	protected static float[][] cFloat(double[][] value)
	{
		if(value == null) return null;
		float[][] arr = new float[value.length][];
		for(int i=0; i < value.length; i++)
		{
			arr[i] = cFloat(value[i]);
		}
		return arr;
	}

	public static int hash(double[] obj)
	{
		if (obj == null) return 0;
		return hashCode(obj);
	}
	public static int hash(double[][] obj)
	{
		if (obj == null) return 0;
		return hashCode(obj);
	}
	
	public static int hashCode(double value)
	{
		return  (int) (  Double.doubleToLongBits(value) ^  (Double.doubleToLongBits(value) >>> 32) );
	}
	
	public static int hashCode(double[] values)
	{
		if (values == null) return 0;
		int result = 17;
		
		for(double value : values)
		{
			result = 31 * hashCode(value)  + result;
		}
		
		return result;
	}
	
	public static int hashCode(double[][] values)
	{
		if (values == null) return 0;
		int result = 11;
		for(double[] value : values)
		{
			result = 37 * hashCode(value) + result; 
		}
		
		return result;
	}
	
	
	public static int hashCode(float value)
	{
		return ( (int) Float.floatToIntBits(value));
	}
	
	public static int hashCode(float[] values)
	{
		if (values == null) return 0;
		int result = 17;
		
		for(float value : values)
		{
			result = 31 * hashCode(value)  + result;
		}
		
		return result;
	}
	
	public static int hashCode(float[][] values)
	{
		if (values == null) return 0;
		int result = 11;
		for(float[] value : values)
		{
			result = 37 * hashCode(value) + result; 
		}
		
		return result;
	}
	
	
	
	
	public static int hashCode(int[] values)
	{
		if (values == null) return 0;
		int result = 53;
		for(int value : values)
		{
			result = 41 * value + result;  
		}
		
		return result;
	}
	

	public static int hashCode(int[][] values)
	{
		if (values == null) return 0;
		int result = 59;
		for(int[] value : values)
		{
			result = 3 * hashCode(value) + result;
		}
		return result;
	}
	
}
