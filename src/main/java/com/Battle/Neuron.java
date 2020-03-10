package com.Battle;

import java.util.ArrayList;
import java.util.Random;
import java.util.Vector;

public class Neuron {
	
	private static double eta = 0.15;
	/**
	 * alpha is the value from the q-learning equation that determines
	 * how quickly the snake learns. It basically discounts part of the 
	 * previous value, but not the whole value.
	 * */
	private static double alpha = 0.5;
	/**
	 * The output value is the value the neuron ends up sending to the next layer.
	 * */
	private double outputVal;
	/**
	 * The outputWeights is an ArrayList of Connections that each have a weight and a 
	 * delta weight. These represent each of the connections between this neuron and 
	 * each neuron in the next layer.
	 * */
	ArrayList<Connection> outputWeights;
	/**
	 * The index corresponds to which Neuron it is in the layer,
	 * like the first, second ... 814th, etc.
	 * */
	private int index;
	/**
	 * The gradient is the amount that this neuron's output value needs to
	 * change by given the error from the layer in front of it.
	 * The gradient change will be achieved by changing weights. 
	 * */
	private double gradient;
	/**
	 * This is the constructor for the Neuron object. It creates a connection
	 * between itself and all of the neurons it will be connected to, and 
	 * it stores this neuron's number in the layer as an instance variable.
	 * 
	 * @param numOutputs is the number of Neurons this neuron will
	 * be connected to in the next layer.
	 * @param myIndex is the position in the layer of this neuron.
	 * */
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
	/**
	 * This is a setter for the output value of this Neuron.
	 * 
	 * @param outputValue is the output value you want this Neuron to have.
	 * */
	public void setOutputValue(double outputValue) {
		this.outputVal = outputValue;
	}
	/**
	 * This is a getter for the output value of this neuron.
	 * 
	 * @return the output value for this Neuron right now.
	 * */
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
	/**
	 * This method calculates how much different the value of this neuron needs to be
	 * compared to what it was. In the context of Battlesnake, this function will be called
	 * to determine what the gradient of the output layer should be based on which of the three
	 * outputs it selected (left, right, forward) compared to what it should have been.
	 * 
	 * For example, if the snake died, the turn after a right turn given a certain board
	 * state, then the output for the "turn right" neuron should have been zero
	 * given that board state.
	 * 
	 * @param targetValues is the value that this Neuron should be according to a
	 * neuron in the layer ahead of it.
	 * */
	public void calcOutputGradients(double targetValues) {
		double delta = targetValues - outputVal;
		/*TODO CHECK LINE BELOW - specifically this operator*/
		this.gradient = delta * this.transferFunctionDerivative(outputVal);
	}
	/**
	 * This method calculates the gradients for the hidden layers. It takes the derivative 
	 * of the weights and squishes it based on the hyperbolic tangent function
	 * (to keep the value between -1 and 1).
	 * */
	public void calcHiddenGradients(final ArrayList<Neuron> nextLayer) {
		double dow = sumDOW(nextLayer);
		/*TODO CHECK LINE BELOW - specifically this operator*/
		this.gradient = dow * this.transferFunctionDerivative(outputVal);
	}
	/**
	 * This method updates the input weights for the current Neuron by 
	 * going through each of its linked output connections one by one, and 
	 * adjusting its weight and its delta weight. 
	 * 
	 * @param prevLayer is the layer behind this neuron's layer.
	 * */
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
