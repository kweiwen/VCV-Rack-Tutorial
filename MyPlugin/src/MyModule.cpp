#include "plugin.hpp"
#include "TPTSVF.h"

#include <algorithm>
#include <cmath>

// export RACK_DIR=/Users/kweiwentseng/Downloads/Rack-SDK
// echo $RACK_DIR
// cd /Users/kweiwentseng/Downloads/MyPlugin/


constexpr float MIN_CUTOFF_HZ = 20.f;
constexpr float MAX_CUTOFF_HZ = 20000.f;
constexpr float TWO_PI = 6.28318530717958647692f;

struct MyModule : Module
{
	TPTSVF filter;
	float lfoPhase = 0.f;

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
		configLight(BLINK_LIGHT, "Cutoff modulation");

		configBypass(INPUT1, OUTPUT1);
	}

	void onReset(const ResetEvent& e) override
	{
		Module::onReset(e);
		filter = TPTSVF{};
		lfoPhase = 0.f;
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override
	{
		Module::onSampleRateChange(e);
		filter = TPTSVF{};
	}

	void process(const ProcessArgs& args) override
	{
		lfoPhase += params[POT3].getValue() * args.sampleTime;
		lfoPhase -= std::floor(lfoPhase);
		const float lfo = std::sin(TWO_PI * lfoPhase);

		// External cutoff CV replaces the internal LFO when connected.
		const float modulation = inputs[INPUT2].isConnected()
			? clamp(inputs[INPUT2].getVoltage() / 5.f, -1.f, 1.f)
			: lfo;
		const float highestCutoff = std::min(MAX_CUTOFF_HZ, args.sampleRate * 0.45f);
		const float cutoff = clamp(
			params[POT1].getValue() * std::exp2(modulation * params[POT2].getValue()),
			MIN_CUTOFF_HZ,
			highestCutoff);
		filter.setParams(cutoff, params[POT4].getValue(), args.sampleRate);

		// // Restore mono outputs after Rack's bypass may have passed polyphonic audio.
		// for (auto& output : outputs)
		// 	output.setChannels(1);
		outputs[OUTPUT1].setVoltage(filter.process(inputs[INPUT1].getVoltage()));
		outputs[OUTPUT2].setVoltage(modulation * 5.f);
		// for (int outputId = OUTPUT3; outputId < OUTPUTS_LEN; ++outputId)
			// outputs[outputId].setVoltage(0.f);
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
