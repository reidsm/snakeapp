package com.Battle;

import java.util.ArrayList;

public class Net {

	private ArrayList< ArrayList<Neuron> > layers;
	private double error;
	private double recentAverageError;
	private static double recentAverageSmoothingFactor = 100.0;
	
	public Net(final ArrayList<Integer> topology) {
		this.error = 0;
		this.recentAverageError = 0;
		int numLayers = topology.size();
		
		for(int layerNum = 0; layerNum < numLayers; ++layerNum) {
			//TODO check this line below
			layers.add(new ArrayList<Neuron>());
			
			int numOutputs = layerNum == topology.size() - 1 ? 0 : topology.get(layerNum + 1);
			
			for(int neuronNum = 0; neuronNum <= topology.get(layerNum); ++neuronNum) {
				//TODO check below
				int layersSize = layers.size();
				ArrayList<Neuron> lastLayer = layers.get(layersSize - 1);
				
				lastLayer.add(new Neuron(numOutputs, neuronNum));
			}
			int layersSize = layers.size();
			ArrayList<Neuron> lastLayer = layers.get(layersSize - 1);
			int lastLayerSize = lastLayer.size();
			Neuron lastNeuron = lastLayer.get(layersSize - 1);
			lastNeuron.setOutputValue(1.0);
		}
	}
	
	public void feedForward(final ArrayList<Double> inputVals) {
		if(inputVals.size() == layers.get(0).size() - 1) {
			
			for(int i = 0; i < inputVals.size(); ++i) {
				layers.get(0).get(i).setOutputValue(inputVals.get(i));
			}
			
			for(int layerNum = 1; layerNum < layers.size(); ++layerNum) {
				ArrayList<Neuron> prevLayer = layers.get(layerNum - 1);
				
				for(int n = 0; n < layers.get(layerNum).size() - 1; ++n) {
					layers.get(layerNum).get(n).feedForward(prevLayer);
				}
			}
			
		} else {
			System.out.println("Error - The size of the inputVals array is different from"
					+ " the size of the first layer");
		}
		
	}
	
	public void backProp(final ArrayList<Double> targetVals) {
		//TODO check 3 lines below
		int layersSize = layers.size();
		ArrayList<Neuron> outputLayer = layers.get(layersSize - 1);
		int outputLayerLastIndex = outputLayer.size() - 1;
		
		for(int i = 0; i < outputLayerLastIndex; ++i) {
			double delta = targetVals.get(i) - outputLayer.get(i).getOutputValue();
			this.error += delta * delta;
		}
		
		this.error /= outputLayerLastIndex;
		this.error = Math.sqrt(this.error);
		
		this.recentAverageError = (this.recentAverageError * this.recentAverageError + this.error) /
				(this.recentAverageSmoothingFactor + 1.0);
		
		for(int j = 0; j < outputLayerLastIndex; ++j) {
			outputLayer.get(j).calcOutputGradients(targetVals.get(j));
		}
		
		for(int layerNum = layersSize - 2; layerNum > 0; --layerNum) {
			ArrayList<Neuron> hiddenLayer = layers.get(layerNum);
			ArrayList<Neuron> nextLayer = layers.get(layerNum + 1);
			
			for(int n = 0; n < hiddenLayer.size(); ++n) {
				hiddenLayer.get(n).calcHiddenGradients(nextLayer);
			}
		}
		
		for(int layerNum = layers.size() - 1; layerNum > 0; --layerNum) {
			ArrayList<Neuron> currLayer = layers.get(layerNum);
			ArrayList<Neuron> prevLayer = layers.get(layerNum - 1);
			
			for(int m = 0; m < currLayer.size() - 1; ++m) {
				currLayer.get(m).updateInputWeights(prevLayer);
			}
		}
		
	}
	
	public void getResults(ArrayList<Double> resultVals) {
		resultVals.clear();
		
		int layersSize = layers.size();
		ArrayList<Neuron> lastLayer = layers.get(layersSize - 1);
		int lastLayerSize = lastLayer.size();
		
		for(int i = 0; i < lastLayerSize - 1; ++i) {
			resultVals.add(lastLayer.get(i).getOutputValue());
		}
	}
	
	public double getRecentAverageError() {
		return this.recentAverageError;
	}
	
}
