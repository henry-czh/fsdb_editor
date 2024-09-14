
drop view if exists v_datFlit;
create view v_datFlit as
select datFlit.*, datOpcode.id as opcode_id
from datFlit, datOpcode
where datFlit.opcode=datOpcode.name
order by time;

drop view if exists v_reqFlit;
create view v_reqFlit as
select reqFlit.*, reqOpcode.id as opcode_id
from reqFlit, reqOpcode
where reqFlit.opcode=reqOpcode.name
order by time;

drop view if exists v_rspFlit;
create view v_rspFlit as
select rspFlit.*, rspOpcode.id as opcode_id
from rspFlit, rspOpcode
where rspFlit.opcode=rspOpcode.name
order by time;

drop view if exists v_snpFlit;
create view v_snpFlit as
select SnpFlitT.*, snpOpcode.id as opcode_id
from SnpFlitT, snpOpcode
where SnpFlitT.opcode=snpOpcode.name
order by time;

