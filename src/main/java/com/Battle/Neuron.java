package com.Battle;

import java.util.ArrayList;
import java.util.Random;
import java.util.Vector;

public class Neuron {
	
	private static double eta = 0.15;
	private static double alpha = 0.5;
	private double outputVal;
	ArrayList<Connection> outputWeights;
	private int index;
	private double gradient;

	public Neuron(int numOutputs, int myIndex) {
		this.gradient = 0;
		this.outputVal = 0;
		
		for(int i = 0; i < numOutputs; ++i) {
			
			//TODO check below plz
			Connection c = new Connection();
			c.setWeight(randomWeight());
			outputWeights.add(c);
		}
		this.index = myIndex;
	}
	
	public void setOutputValue(double outputValue) {
		this.outputVal = outputValue;
	}
	
	public double getOutputValue() {
		return this.outputVal;
	}
	
	public void feedForward(final ArrayList<Neuron> prevLayer) {
		double sum = 0.0;
		for(int i = 0; i < prevLayer.size(); ++i) {
			sum += prevLayer.get(i).getOutputValue() * prevLayer.get(i).outputWeights.get(index).getWeight();
		}
		outputVal = this.transferFunction(sum);
	}
	
	public void calcOutputGradients(double targetValues) {
		double delta = targetValues - outputVal;
		/*TODO CHECK LINE BELOW - specifically this operator*/
		this.gradient = delta * this.transferFunctionDerivative(outputVal);
	}
	
	public void calcHiddenGradients(final ArrayList<Neuron> nextLayer) {
		double dow = sumDOW(nextLayer);
		/*TODO CHECK LINE BELOW - specifically this operator*/
		gradient = dow * this.transferFunctionDerivative(outputVal);
	}
	
	public void updateInputWeights(ArrayList<Neuron> prevLayer) {
		for(int i = 0; i < prevLayer.size(); ++i) {
			Neuron currentNeuron = prevLayer.get(i);
			
			double oldDeltaWeight = currentNeuron.outputWeights.get(index).getDeltaWeight();
			double newDeltaWeight = eta * currentNeuron.getOutputValue() * this.gradient + alpha * oldDeltaWeight;
			
			currentNeuron.outputWeights.get(index).setDeltaWeight(newDeltaWeight);
			
			double newWeight = currentNeuron.outputWeights.get(index).getWeight() + newDeltaWeight;
			
			currentNeuron.outputWeights.get(index).setWeight(newWeight);
			
		}
	}
	
	private static double transferFunction(double x) {
		return Math.tanh(x);
	}
	
	private static double transferFunctionDerivative(double x) {
		return 1.0 - x * x;
	}
	
	private static double randomWeight() {
		Random rand = new Random();
		double randDouble = rand.nextDouble();
		return randDouble;
	}
	
	private double sumDOW(final ArrayList<Neuron> nextLayer) {
		double sum = 0.0;
		for(int i = 0; i < nextLayer.size() - 1; ++i) {
			sum += outputWeights.get(i).getWeight() * nextLayer.get(i).getGradient();
		}
		return sum;
	}
	
	private double getGradient() {
		return this.gradient;
	}
}
