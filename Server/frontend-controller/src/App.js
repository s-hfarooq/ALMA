import React, { Component } from 'react';
import 'bootstrap/dist/css/bootstrap.min.css';
import './App.css';
import { changeColor } from './services/additionalFunctions'
import { ChromePicker, SketchPicker } from 'react-color';

class App extends React.Component {

  state = {
    background: '#fff'
  }

  // runs once color stops changing
  handleChangeComplete = (color) => {
    this.setState({ background: color.hex });
    let newColStr = "col " + color.rgb.r + " " + color.rgb.g + " " + color.rgb.b;
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

  render() {

    return (
      <SketchPicker
        color={ this.state.background }
        onChangeComplete={ this.handleChangeComplete }
        // onChange={ this.handleChange }
      />
    );
  }
}

export default App;
