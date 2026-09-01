#include "plugin.hpp"
#include "FilterDesigner.h"

// export RACK_DIR=/Users/kweiwentseng/Downloads/Rack-SDK
// echo $RACK_DIR
// cd /Users/kweiwentseng/Downloads/MyPlugin/

constexpr float MIN_CUTOFF_HZ = 20.f;
constexpr float MAX_CUTOFF_HZ = 20000.f;

struct MyModule : Module
{
	float phase_lfo = 0.f;
	float intPart_lfo = 0.f;
	FilterDesigner filterDesigner;
	float input1 = 0.f;
	float input2 = 0.f;
	float output1 = 0.f;
	float output2 = 0.f;

	enum ParamId
	{
		POT1,	// 0
		POT2,   // 1
		POT3,   // 2
		POT4,   // 3
		POT5,   // 4
		POT6,   // 5
		PARAMS_LEN	 // 6
	};
	enum InputId
	{
		INPUT1,
		INPUT2,
		INPUT3,
		INPUTS_LEN
	};
	enum OutputId
	{
		OUTPUT1,
		OUTPUT2,
		OUTPUT3,
		OUTPUT4,
		OUTPUT5,
		OUTPUT6,
		OUTPUTS_LEN
	};
	enum LightId
	{
		BLINK_LIGHT,
		LIGHTS_LEN
	};

	MyModule()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(POT1, MIN_CUTOFF_HZ, MAX_CUTOFF_HZ, 1000.f, "Cutoff frequency", " Hz");
		configParam(POT2, 0.f, 5.f, 1.f, "Modulation depth", " octaves");
		configParam(POT3, 0.05f, 20.f, 1.f, "Internal modulation rate", " Hz");
		configParam(POT4, 0.025f, 40.f, 0.707f, "Resonance(Q)", "");

		configInput(INPUT1, "Audio");
		configInput(INPUT2, "Cutoff modulation");
		configOutput(OUTPUT1, "Low-pass filter");
		configOutput(OUTPUT2, "Modulation");

		filterDesigner.model = E_LOW_PASS_2;
	}

	void process(const ProcessArgs &args) override
	{
		const float lfoRate = params[POT3].getValue();
		phase_lfo = phase_lfo + lfoRate * args.sampleTime;
		phase_lfo = std::modf(phase_lfo, &intPart_lfo);
		const float lfo = std::sin(phase_lfo * 2.f * M_PI);
		
		const float q = params[POT4].getValue();

		// INPUT2 expects a bipolar -5 V to +5 V modulation signal. When it is
		// unplugged, the internal LFO is used so the cutoff still moves.
		const float modulation = inputs[INPUT2].isConnected()
			? clamp(inputs[INPUT2].getVoltage() / 5.f, -1.f, 1.f)
			: lfo;
		const float modulationDepth = params[POT2].getValue();
		const float baseCutoff = params[POT1].getValue();
		const float highestCutoff = std::min(MAX_CUTOFF_HZ, args.sampleRate * 0.45f);
		const float cutoff = clamp(
			baseCutoff * std::pow(2.f, modulation * modulationDepth),
			MIN_CUTOFF_HZ,
			highestCutoff);

		// Filter Designer returns {a0, a1, a2, b0, b1, b2}. Its coefficients
		// are normalized to b0 = 1, so this is the Direct Form I equation.
		filterDesigner.setParameter(cutoff, args.sampleRate, q);
		const float *coefficients = filterDesigner.getCoefficients();
		const float input = inputs[INPUT1].getVoltage();
		const float filtered = coefficients[0] * input
			+ coefficients[1] * input1
			+ coefficients[2] * input2
			- coefficients[4] * output1
			- coefficients[5] * output2;

		input2 = input1;
		input1 = input;
		output2 = output1;
		output1 = filtered;

		outputs[OUTPUT1].setVoltage(filtered);
		outputs[OUTPUT2].setVoltage(modulation * 5.f);
		lights[BLINK_LIGHT].setBrightness((modulation + 1.f) * 0.5f);
	}
};

struct MyModuleWidget : ModuleWidget
{
	MyModuleWidget(MyModule *module)
	{
		setModule(module);

		// Panel
		setPanel(createPanel(asset::plugin(pluginInstance, "res/MyModule.svg")));
		
		// Screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec( 5.24, 46.063)), module, MyModule::POT1));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 46.063)), module, MyModule::POT2));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(25.24, 46.063)), module, MyModule::POT3));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec( 5.24, 61.063)), module, MyModule::POT4));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 61.063)), module, MyModule::POT5));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(25.24, 61.063)), module, MyModule::POT6));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec( 5.24, 77.478)), module, MyModule::INPUT1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 77.478)), module, MyModule::INPUT2));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.24, 77.478)), module, MyModule::INPUT3));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec( 5.24, 100)), module, MyModule::OUTPUT1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 100)), module, MyModule::OUTPUT2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.24, 100)), module, MyModule::OUTPUT3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec( 5.24, 110)), module, MyModule::OUTPUT4));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 110)), module, MyModule::OUTPUT5));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.24, 110)), module, MyModule::OUTPUT6));



		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(15.24, 25.81)), module, MyModule::BLINK_LIGHT));
	}
};

Model *modelMyModule = createModel<MyModule, MyModuleWidget>("MyModule");
