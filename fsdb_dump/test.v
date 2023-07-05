module test();

reg clk,rstn;

reg  [7:0] a;
wire [7:0] b;

always #5 clk = ~clk;

initial begin
    clk = 0;
    rstn = 0;
	a = 0;
    #10
    rstn = 1;
    #10
    a = 8'hf;
    #20
    a = 8'ha;
	#100
    $finish;
end

dut_0 dut0(
    .clk    (clk),
    .rstn   (rstn),
    .a_dut0  (a),
    .b_dut0  (b)
);

dut_1 dut1(
    .clk    (clk),
    .rstn   (rstn),
    .a_dut1 (a)
);

endmodule

module dut_1 (
    input clk,rstn,
    input a_dut1
);

reg c;

always @(posedge clk or negedge rstn)
if(!rstn)
    c <= 'b0;
else
    c <= a_dut1;

endmodule

module dut_0(
    input clk,rstn,
    input a_dut0,
    output b_dut0
);

wire clk, rstn;
wire [7:0] a_dut;

reg [7:0] b_dut;

always @(posedge clk or negedge rstn)
	if(~rstn)
		b_dut0 <= 8'h00;
	else
		b_dut0 <= a_dut0;

endmodule
