import React, { Component, Fragment } from 'react';
import 'bootstrap/dist/css/bootstrap.min.css';
import { changeColor, lightOptions } from './services/additionalFunctions'
import { ChromePicker, SketchPicker, PhotoshopPicker } from 'react-color';
import Select from 'react-select';


class App extends React.Component {

  state = {
    background: '#fff',
    selectedOption: "off"
  }

  // runs once color stops changing
  handleChangeComplete = (color) => {
    this.setState({ background: color.hex });
    let option = this.state.selectedOption.value;
    let newColStr = "off"
    if(option === "col" || option === "col2")
      newColStr = option + " " + color.rgb.r + " " + color.rgb.g + " " + color.rgb.b;
    else if(option === "off" || option === "fade")
      newColStr = option;
    else if(option === "both")
      newColStr = "colB " + color.rgb.r + " " + color.rgb.g + " " + color.rgb.b;
    console.log(newColStr);
    changeColor(newColStr);
  };

  // runs everytime a color changes
  handleChange(color, event) {
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
    let newColStr = "col " + color.rgb.r + " " + color.rgb.g + " " + color.rgb.b;
    console.log(newColStr);
    changeColor(newColStr);
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
          onChangeComplete={ this.handleChangeComplete }
          // onChange={ this.handleChange }
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
