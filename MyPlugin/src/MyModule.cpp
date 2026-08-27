#include "plugin.hpp"

// export RACK_DIR=/Users/kweiwentseng/Downloads/Rack-SDK
// echo $RACK_DIR
// cd /Users/kweiwentseng/Downloads/MyPlugin/

struct MyModule : Module
{
	float phase1 = 0.f;
	float intPart1 = 0.f;

	float phase2 = 0.f;
	float intPart2 = 0.f;

	float lfo_phase = 0.f;
	float lfo_intPart = 0.f;

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
		configParam(POT1, 0.f, 1.f, 0.f, "");
		configInput(INPUT1, "");
		configOutput(OUTPUT1, "");
		configOutput(OUTPUT2, "");
		configOutput(OUTPUT3, "");
	}

	void process(const ProcessArgs &args) override
	{	
		// Potentiometer
		// Get Pote's value
		float pitch1 = params[POT1].getValue(); // 0 ~ 1
		float pitch2 = params[POT2].getValue(); // 0 ~ 1
		float pitch3 = params[POT3].getValue(); // 0 ~ 1
		// 1. params是一個數組
		// 2. params[POT2]代表取數組裡面的元素（他是一個抽象的旋鈕物件，裡面有很組件）
		// 3. params[POT2].getValue() 這個組件，裡面有函數成員叫`getValue()`，他的功能是，呼叫以後，返回`旋鈕的值`
		// 4. 將返回`旋鈕的值`，賦予給變量`lfo_pitch`

		// Get CV Jack's value
		float cv = inputs[INPUT1].getVoltage();
		float brightness = cv / 10.f;
		lights[BLINK_LIGHT].setBrightness(brightness);

		// AM Amplitude Modulation

		// from here
		// set freq = 0.05f Hz
		// lfo_freq = 0 ~ 20
		lfo_phase = lfo_phase + lfo_freq * args.sampleTime;
		lfo_phase = std::modf(lfo_phase, &lfo_intPart);
		// to here

		float lfo = std::sin(2.f * M_PI * lfo_phase); 

		// FM Frequency Modulation
		phase2 = phase2 + 20.f * pitch3 * args.sampleTime;
		phase2 = std::modf(phase2, &lfo_intPart);
		float fm = std::sin(2.f * M_PI * phase2); 

        float freq = dsp::FREQ_C4 * std::pow(2.f, pitch1+cv); // `picth to frequency`, which is similar to `mtof`
        // phase = phase + freq * args.sampleTime; // which is equivalent to `phase += args.sampleTime;` 
		// phase = phase + 1 / args.sampleRate;
		// phase = phase + 1 / 44100;
		
		// from here...
		phase1 = phase1 + freq * args.sampleTime;
        phase1 = std::modf(phase1, &intPart1); // take fractional part only...
		// to here...
        
		// equiv to cos~...
		float sine = std::sin(2.f * M_PI * phase1); // phase1 = 0 ~ 1 -> 0 ~ 2pi -> sin(x)
		
		outputs[OUTPUT1].setVoltage(phase1);
		outputs[OUTPUT2].setVoltage(sine);
		outputs[OUTPUT3].setVoltage(sine * lfo);

		// SAWTOOTH
		outputs[OUTPUT4].setVoltage((phase1 - 0.5f) * 2.f);
		// SQUARE
		float square = 0.f;
		if (sine >= 0.f)
		{
			square = 1.f;
		}
		else
		{
			square = -1.f;
		}
		outputs[OUTPUT5].setVoltage(square);
		// TRIANGLE
		outputs[OUTPUT6].setVoltage(std::asin(sine) * 2 / M_PI);

		// 1 block 64/128/256/512/1024 sample
		// 1st block
		// phase = 0 + 1/44100; // 1st
		// phase = 1/44100 + 1/44100 // 2nd
		// phase = 2/44100 + 1/44100 // 3rd
		// ...
		// phase = 63/44100 + 1/44100 // 64th
		
		// 2nd 
		// phase = 65/44100 + 1/44100; // 1st
		// phase = 66/44100 + 1/44100 // 2nd
		// phase = 67/44100 + 1/44100 // 3rd
		// ...
		// phase = 127/44100 + 1/44100 // 64th
				
		// float square;
		// if (phase < 0.5f) 
		// {
		// 	square = 1.f;
		// } 
		// else 
		// {
		// 	square = -1.f;
		// }

		// float sawtooth;
		// sawtooth = (phase - 0.5f) * 2.f; // 0 ~ 1 > -0.5 ~ 0.5 > -1 ~ 1
		
		// float harmonics_1st = std::sin(2.f * M_PI * phase1 * freq) * 1.0f;
		// float harmonics_2nd = std::sin(2.f * M_PI * phase1 * freq * 2.f) / (2.f * M_PI);
		// float harmonics_3rd = std::sin(2.f * M_PI * phase1 * freq * 3.f) / (3.f * M_PI);
		// float harmonics_4th = std::sin(2.f * M_PI * phase1 * freq * 4.f) / (4.f * M_PI);
		// float harmonics_5th = std::sin(2.f * M_PI * phase1 * freq * 5.f) / (5.f * M_PI);
		// float fourier_series = harmonics_1st + harmonics_2nd + harmonics_3rd + harmonics_4th + harmonics_5th;
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