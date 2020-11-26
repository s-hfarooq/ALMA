import React, { Component, Fragment } from 'react';
import 'bootstrap/dist/css/bootstrap.min.css';
import { connectChanger, endConnection, changeColor, lightOptions } from './services/additionalFunctions'
import { ChromePicker, SketchPicker, PhotoshopPicker } from 'react-color';
import Select from 'react-select';


class App extends React.Component {

  state = {
    background: '#fff',
    selectedOption: "off",
    isConnected: false,
  }

  // runs everytime a color changes
  handleChange = async (color, event) => {
    // color = {
    //   hex: '#333',
    //   rgb: {
    //     r: 51,
    //     g: 51,
    //     b: 51,
    //     a: 1,
    //   },
    //   hsl: {
    //     h: 0,
    //     s: 0,
    //     l: .20,
    //     a: 1,
    //   },
    // }

    this.setState({ background: color.hex });
    let option = this.state.selectedOption.value;
    let newColStr = ""
    if(option === "col" || option === "col2")
      newColStr = option + " " + color.rgb.r + " " + color.rgb.g + " " + color.rgb.b;
    else if(option === "off" || option === "fade")
      newColStr = option;
    else if(option === "both")
      newColStr = "colB " + color.rgb.r + " " + color.rgb.g + " " + color.rgb.b;
    console.log(newColStr);
    if(!this.state.isConnected) {
      console.log("starting connection")
      await connectChanger(newColStr);
      console.log("connected");
      this.setState({ isConnected: true });
    }

    if(newColStr.length > 1) {
      console.log('sending new col')
      await changeColor(newColStr);
      console.log('sent new col')
    }

    // Probably should kill the connection at some point
    // Also need to figure out what to do when multiple users connect - currently just breaks everything
  }

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
          //onChangeComplete={ this.handleChangeComplete }
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
