var expect = require('chai').expect;

var turf = require('bindings')('turf_file');


describe('Argument Validation', () => {
  it('should throw for wrong number of arguments', () => {

    // expect(turf.pack.bind(turf, )).to.throw("unpack takes one argument");
    // expect(turf.pack.bind(turf, )).to.throw("pack takes two arguments");
    expect(turf.pack.bind(turf, 'a')).to.throw("pack takes two arguments");
    expect(turf.pack.bind(turf, 'a', 'b')).to.throw("pack takes 1st argument array of shapes");
    expect(turf.pack.bind(turf, ['a'], 'b')).to.throw("pack takes 2nd argument array of turfs");
    expect(turf.pack.bind(turf, [], ['b'])).to.throw("Array lengths don't match");
    expect(turf.pack.bind(turf, ['a'], ['b'])).to.throw("pack takes 1st argument array of arrays");
    expect(turf.pack.bind(turf, [['a']], ['b'])).to.throw("pack takes 1st argument array of arrays of floats");

  });
});
