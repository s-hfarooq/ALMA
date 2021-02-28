import React from 'react';
import { sendCommand, lightOptions } from './services/additionalFunctions'
import { ChromePicker } from 'react-color';
import Select from 'react-select';
import Button from 'react-bootstrap/Button';
import InputNumber from 'rc-input-number';

class App extends React.Component {
  state = {
    background: '#fff',
    selectedOption: { value: "all" },
    fadeSpeed: 50,
  }

  // Runs everytime a color changes
  handleChange = async (color, event) => {
    this.setState({ background: color.hex });
  }

  // Kill connection when colors aren't being changed
  handleChangeComplete = async (color, event) => {
    let option = this.state.selectedOption.value;
    let newColStr = color.rgb.r + "-" + color.rgb.g + "-" + color.rgb.b;

    // Figure out what output string ending needs to be
    switch(option) {
      case "1colceiling":
        newColStr += "-1-1-0-";
        break;
      case "1colcouch":
        newColStr += "-1-2-0-";
        break;
      case "2colceiling":
        newColStr += "-2-1-0-";
        break;
      case "2colcouch":
        newColStr += "-2-2-0-";
        break;
      case "bothceiling":
        newColStr += "-0-1-0-";
        break;
      case "bothcouch":
        newColStr += "-0-2-0-";
        break;
      default:
        newColStr += "-0-0-0-";
    }

    await sendCommand(newColStr);
  }

  // Handle strip selection dropdown menu
  handleSelectChange = selectedOption => {
    this.setState(
      { selectedOption },
      () => console.log(`Option selected:`, this.state.selectedOption)
    );
  };

  onNumChange = fadeSpeed => {
    this.setState({fadeSpeed});
  }

  render() {
    return (
      <div>
        <center>
          <ChromePicker
            color = { this.state.background }
            onChangeComplete = { this.handleChangeComplete }
            onChange = { this.handleChange }
          />

          <br/>

          <Select
              className = "basic-single"
              classNamePrefix = "select"
              defaultValue = {lightOptions[0]}
              isDisabled = {false}
              isLoading = {false}
              isClearable = {false}
              isRtl = {false}
              isSearchable = {false}
              name = "color"
              options = {lightOptions}
              onChange = {this.handleSelectChange}
          />

          <br/>

          <InputNumber min = {10}
                       value = {this.state.fadeSpeed}
                       defaultValue = {50}
                       step = {1}
                       style = {{margin: 10}}
                       onChange = {this.onNumChange}
          />

         <Button variant="outline-dark" onClick={async () => {
              await sendCommand("0-0-0-3-0-" + this.state.fadeSpeed + "-");
          }}>Start Fade (delay above)</Button>
        </center>
      </div>
    );
  }
}

export default App;
