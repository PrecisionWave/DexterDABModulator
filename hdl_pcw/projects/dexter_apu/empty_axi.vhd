

package empty_axi_pkg is
    type t_axi_response is (OKAY, SLVERR, DECERR);
end package;

library ieee;
use ieee.std_logic_1164.all;

library work;
use work.empty_axi_pkg.all;

entity empty_axi is
    generic (
        constant READ_RESPONSE  : t_axi_response := SLVERR;
        constant WRITE_RESPONSE : t_axi_response := SLVERR
    );
    port (
        S_AXI_ACLK    : in  std_logic;
        S_AXI_ARESETN : in  std_logic;

        S_AXI_AWVALID : in  std_logic;
        S_AXI_AWREADY : out std_logic;

        S_AXI_WVALID  : in  std_logic;
        S_AXI_WREADY  : out std_logic;

        S_AXI_BVALID  : out std_logic;
        S_AXI_BREADY  : in  std_logic;
        S_AXI_BRESP   : out std_logic_vector(1 downto 0);

        S_AXI_ARVALID : in  std_logic;
        S_AXI_ARREADY : out std_logic;

        S_AXI_RVALID  : out std_logic;
        S_AXI_RREADY  : in  std_logic;
        S_AXI_RRES    : out std_logic_vector(1 downto 0)
    );
end entity;


architecture rtl of empty_axi is
    signal write_pending_aw : std_logic;
    signal write_pending_w  : std_logic;
    signal read_pending_ar  : std_logic;
begin

    -- Write signaling
    S_AXI_AWREADY <= not write_pending_aw;
    S_AXI_WREADY  <= not write_pending_w;
    S_AXI_BVALID  <= write_pending_w and write_pending_aw;
    with WRITE_RESPONSE select
        S_AXI_BRESP <=
        "10" when SLVERR,
        "00" when OKAY,
        "11" when DECERR,
        "10" when others;

    -- Read signaling
    S_AXI_ARREADY <= not read_pending_ar;
    S_AXI_RVALID  <= read_pending_ar;
    with READ_RESPONSE select
        S_AXI_RRES <=
        "10" when SLVERR,
        "00" when OKAY,
        "11" when DECERR,
        "10" when others;

    -- State machine
    process (S_AXI_ACLK, S_AXI_ARESETN) begin
        if S_AXI_ARESETN = '0' then
            write_pending_aw <= '0';
            write_pending_w  <= '0';
            read_pending_ar  <= '0';
        elsif rising_edge(S_AXI_ACLK) then
            if write_pending_aw = '0' and S_AXI_AWVALID = '1' then
                write_pending_aw <= '1';
            end if;

            if write_pending_w = '0' and S_AXI_WVALID = '1' then
                write_pending_w <= '1';
            end if;

            if read_pending_ar = '0' and S_AXI_ARVALID = '1' then
                read_pending_ar <= '1';
            end if;

            if read_pending_ar = '1' and S_AXI_RREADY = '1' then
                read_pending_ar <= '0';
            end if;

            if write_pending_aw = '1' and write_pending_w = '1' and S_AXI_BREADY = '1' then
                write_pending_aw <= '0';
                write_pending_w  <= '0';
            end if;
        end if;
    end process;

end architecture;