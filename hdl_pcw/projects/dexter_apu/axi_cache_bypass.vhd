library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

entity axi_cache_bypass is
    port (
        aclk             : in  std_logic;
        arstn            : in  std_logic;

        M_AXI_DP_ARADDR  : out std_logic_vector(30 downto 0);
        M_AXI_DP_ARPROT  : out std_logic_vector(2 downto 0);
        M_AXI_DP_ARREADY : in  std_logic;
        M_AXI_DP_ARVALID : out std_logic;
        M_AXI_DP_AWADDR  : out std_logic_vector(31 downto 0);
        M_AXI_DP_AWPROT  : out std_logic_vector(2 downto 0);
        M_AXI_DP_AWREADY : in  std_logic;
        M_AXI_DP_AWVALID : out std_logic;
        M_AXI_DP_BREADY  : out std_logic;
        M_AXI_DP_BRESP   : in  std_logic_vector(1 downto 0);
        M_AXI_DP_BVALID  : in  std_logic;
        M_AXI_DP_RDATA   : in  std_logic_vector(31 downto 0);
        M_AXI_DP_RREADY  : out std_logic;
        M_AXI_DP_RRESP   : in  std_logic_vector(1 downto 0);
        M_AXI_DP_RVALID  : in  std_logic;
        M_AXI_DP_WDATA   : out std_logic_vector(31 downto 0);
        M_AXI_DP_WREADY  : in  std_logic;
        M_AXI_DP_WSTRB   : out std_logic_vector(3 downto 0);
        M_AXI_DP_WVALID  : out std_logic;

        S_AXI_DP_ARADDR  : in  std_logic_vector(30 downto 0);
        S_AXI_DP_ARPROT  : in  std_logic_vector(2 downto 0);
        S_AXI_DP_ARREADY : out std_logic;
        S_AXI_DP_ARVALID : in  std_logic;
        S_AXI_DP_AWADDR  : in  std_logic_vector(31 downto 0);
        S_AXI_DP_AWPROT  : in  std_logic_vector(2 downto 0);
        S_AXI_DP_AWREADY : out std_logic;
        S_AXI_DP_AWVALID : in  std_logic;
        S_AXI_DP_BREADY  : in  std_logic;
        S_AXI_DP_BRESP   : out std_logic_vector(1 downto 0);
        S_AXI_DP_BVALID  : out std_logic;
        S_AXI_DP_RDATA   : out std_logic_vector(31 downto 0);
        S_AXI_DP_RREADY  : in  std_logic;
        S_AXI_DP_RRESP   : out std_logic_vector(1 downto 0);
        S_AXI_DP_RVALID  : out std_logic;
        S_AXI_DP_WDATA   : in  std_logic_vector(31 downto 0);
        S_AXI_DP_WREADY  : out std_logic;
        S_AXI_DP_WSTRB   : in  std_logic_vector(3 downto 0);
        S_AXI_DP_WVALID  : in  std_logic
    );
end entity;

architecture rtl of axi_cache_bypass is

begin

    M_AXI_DP_ARADDR  <= S_AXI_DP_ARADDR(30 downto 0);
    M_AXI_DP_ARPROT  <= S_AXI_DP_ARPROT;
    S_AXI_DP_ARREADY <= M_AXI_DP_ARREADY;
    M_AXI_DP_ARVALID <= S_AXI_DP_ARVALID;
    M_AXI_DP_AWADDR  <= S_AXI_DP_AWADDR;
    M_AXI_DP_AWPROT  <= S_AXI_DP_AWPROT;
    S_AXI_DP_AWREADY <= M_AXI_DP_AWREADY;
    M_AXI_DP_AWVALID <= S_AXI_DP_AWVALID;
    M_AXI_DP_BREADY  <= S_AXI_DP_BREADY;
    S_AXI_DP_BRESP   <= M_AXI_DP_BRESP;
    S_AXI_DP_BVALID  <= M_AXI_DP_BVALID;
    S_AXI_DP_RDATA   <= M_AXI_DP_RDATA;
    M_AXI_DP_RREADY  <= S_AXI_DP_RREADY;
    S_AXI_DP_RRESP   <= M_AXI_DP_RRESP;
    S_AXI_DP_RVALID  <= M_AXI_DP_RVALID;
    M_AXI_DP_WDATA   <= S_AXI_DP_WDATA;
    S_AXI_DP_WREADY  <= M_AXI_DP_WREADY;
    M_AXI_DP_WSTRB   <= S_AXI_DP_WSTRB;
    M_AXI_DP_WVALID  <= S_AXI_DP_WVALID;

end architecture;