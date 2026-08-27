#include "plugin.hpp"
#include "CircularBuffer.h"

// export RACK_DIR=/Users/kweiwentseng/Downloads/Rack-SDK
// echo $RACK_DIR
// cd /Users/kweiwentseng/Downloads/MyPlugin/

constexpr float DELAY_TIME_SECONDS = 0.25f;
// constexpr float FEEDBACK = 0.35f;
// constexpr float DRY_WET = 0.5f;

struct MyModule : Module
{
	float phase_lfo = 0.f;
	float intPart_lfo = 0.f;
	CircularBuffer delayBuffer;
	unsigned int delayInSamples = 1;

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

		configParam(POT1, 1.f, 12000.f, 12000.f, "Delay time", " samples");
		getParamQuantity(POT1)->snapEnabled = true;

		configInput(INPUT1, "");
		configOutput(OUTPUT1, "");
		configOutput(OUTPUT2, "");
		configOutput(OUTPUT3, "");

		configureDelay(48000.f);
	}

	void configureDelay(float sampleRate)
	{
		//=== 48000 * 0.25
		//=== Delay buffer length = 12000
		//=== Maximum delay time is 12000 sample
		delayInSamples = static_cast<unsigned int>(sampleRate * DELAY_TIME_SECONDS);
		if (delayInSamples < 1)
			delayInSamples = 1;

		delayBuffer.createCircularBuffer(delayInSamples + 1);
	}

	void onSampleRateChange(const SampleRateChangeEvent &e) override
	{
		configureDelay(e.sampleRate);
	}

	void process(const ProcessArgs &args) override
	{
		int delayTime = params[POT1].getValue(); // 1 - 12000

		phase_lfo = phase_lfo + 1.f * 20.f * args.sampleTime;
		phase_lfo = std::modf(phase_lfo, &intPart_lfo);
		float lfo = std::sin(phase_lfo * 2.f * M_PI);

		const float input = inputs[INPUT1].getVoltage();
		const float delayedSignal = delayBuffer.readBuffer(delayTime);

		// Feed part of the delayed signal back into the buffer to create repeats.
		delayBuffer.writeBuffer(input);		
		// delayBuffer.writeBuffer(input + delayedSignal * FEEDBACK);
		
		outputs[OUTPUT1].setVoltage(lfo);
		outputs[OUTPUT2].setVoltage(delayedSignal);
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
