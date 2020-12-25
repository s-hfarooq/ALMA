import React, { Component, Fragment } from 'react';
import 'bootstrap/dist/css/bootstrap.min.css';
import { connectChanger, endConnection, changeColor, lightOptions } from './services/additionalFunctions'
import { ChromePicker } from 'react-color';
import Select from 'react-select';

class App extends React.Component {
  state = {
    background: '#fff',
    selectedOption: "col",
    isConnected: false,
  }

  // Runs everytime a color changes
  handleChange = async (color, event) => {
    this.setState({ background: color.hex });
    let option = this.state.selectedOption.value;
    let newColStr = ""

    if(option === "1col" || option === "2col" || option === "both")
      newColStr = color.rgb.r + " " + color.rgb.g + " " + color.rgb.b + " " + option;

    console.log(newColStr);
    if(!this.state.isConnected) {
      console.log("Starting connection")
      connectChanger();
      console.log("Connected");
      this.setState({ isConnected: true });
    }

    if(newColStr.length > 1) {
      console.log('Sending new color')
      changeColor(newColStr);
      console.log('Sent new color')
    }
  }

  // Kill connection when colors aren't being changed
  handleChangeComplete = async (color, event) => {
    if(!this.state.isConnected) {
      console.log("Starting connection")
      await connectChanger();
      console.log("Connected");
      this.setState({ isConnected: true });
    }

    if(this.state.isConnected) {
      let option = this.state.selectedOption.value;
      let newColStr = ""

      if(option === "1col" || option === "2col" || option === "both")
        newColStr = color.rgb.r + " " + color.rgb.g + " " + color.rgb.b + " " + option;

      // Ensure color is set - just send same command 5 times
      // Bad way to do it - need to figure out better method later
      for(let i = 0; i < 5; i++)
        changeColor(newColStr);

      endConnection();
      this.setState({ isConnected: false });
    }
  }

  // Handle strip selection dropdown menu
  handleSelectChange = selectedOption => {
    this.setState(
      { selectedOption },
      () => console.log(`Option selected:`, this.state.selectedOption)
    );
  };

  render() {
    return (
      <div>
        <ChromePicker
          color={ this.state.background }
          onChangeComplete={ this.handleChangeComplete }
          onChange={ this.handleChange }
        />

        <br/>

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
      </div>
    );
  }
}

export default App;
