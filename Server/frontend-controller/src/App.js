import React from 'react';
import { sendCommand, getCommandString, lightOptions, animationOptions, getAnimationString } from './services/additionalFunctions'
import { ChromePicker } from 'react-color';
import Select from 'react-select';
import Button from 'react-bootstrap/Button';
import InputNumber from 'rc-input-number';

class App extends React.Component {
    state = {
        background: '#fff',
        selectedOption: { value: "ceiling" },
        fadeSpeed: 50,
        animationNum: { value: "0" },
    }

    // Runs everytime a color changes
    handleChange = async (color, event) => {
        this.setState({ background: color.hex });
    }

    // Set final color once colors stop changing
    handleChangeComplete = async (color, event) => {
        let option = this.state.selectedOption.value;
        await sendCommand(getCommandString(color, option, false, 0));
    }

    // Handle strip selection dropdown menu
    handleSelectChange = selectedOption => {
        this.setState(
            { selectedOption },
            () => console.log(`Option selected:`, this.state.selectedOption)
        );
    };

    // Handle animamtion selection dropdown menu
    handleAnimationSelect = animationNum => {
        this.setState(
            { animationNum },
            () => console.log(`Option selected:`, this.state.animationNum)
        );
    };

    // Handle fade speed input
    onNumChange = fadeSpeed => {
        this.setState({ fadeSpeed });
    }

    // Render output
    render() {
        return (
            <div> <center>
                {/* Color picker */}
                <ChromePicker
                    color={this.state.background}
                    onChangeComplete={this.handleChangeComplete}
                    onChange={this.handleChange}
                />

                <br/>

                {/* Strip selector */}
                <Select
                    className="basic-single"
                    classNamePrefix="select"
                    defaultValue={lightOptions[0]}
                    isDisabled={false}
                    isLoading={false}
                    isClearable={false}
                    isRtl={false}
                    isSearchable={false}
                    name="color"
                    options={lightOptions}
                    onChange={this.handleSelectChange}
                />

                <br/>

                {/* Fade delay text box input */}
                <InputNumber min={10}
                    value={this.state.fadeSpeed}
                    defaultValue={50}
                    step={1}
                    style={{ margin: 10 }}
                    onChange={this.onNumChange}
                />

                {/* Fade start button */}
                <Button variant="outline-dark" onClick={async () => {
                    // await sendCommand("0-0-0-3-0-" + this.state.fadeSpeed + "-");
                    await sendCommand(getCommandString(0, this.state.selectedOption.value, true, this.state.fadeSpeed));
                }}>Start Fade (delay above)</Button>

                <br/><br/><br/>

                {/* Animation selector */}
                <Select
                    className="basic-single"
                    classNamePrefix="select"
                    defaultValue={animationOptions[0]}
                    isDisabled={false}
                    isLoading={false}
                    isClearable={false}
                    isRtl={false}
                    isSearchable={false}
                    name="animation"
                    options={animationOptions}
                    onChange={this.handleAnimationSelect}
                />

                {/* Animation start button */}
                <Button variant="outline-dark" onClick={async () => {
                    await sendCommand(getAnimationString(this.state.animationNum.value));
                }}>Start Ceiling Animation</Button>
            </center> </div>
        );
    }
}

export default App;
