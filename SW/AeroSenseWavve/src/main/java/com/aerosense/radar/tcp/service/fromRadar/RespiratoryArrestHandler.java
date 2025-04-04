package com.aerosense.radar.tcp.service.fromRadar;

import com.google.common.collect.Sets;
import com.aerosense.radar.tcp.hander.base.AbstractFromRadarProtocolDataHandler;
import com.aerosense.radar.tcp.hander.callback.RadarHandlerCallBack;
import com.aerosense.radar.tcp.protocol.FunctionEnum;
import com.aerosense.radar.tcp.protocol.RadarProtocolData;
import org.springframework.stereotype.Service;

import java.util.Set;

/**
 * @author ：ywb
 * @date ：Created in 2022/1/7 10:35
 * @modified By：
 * <p>
 */
@Service
public class RespiratoryArrestHandler extends AbstractFromRadarProtocolDataHandler {
    public RespiratoryArrestHandler(RadarHandlerCallBack handlerCallBack) {
        super(handlerCallBack);
    }

    @Override
    public Object process(RadarProtocolData protocolData) {
        protocolData.setFunction(FunctionEnum.respiratoryArrest);
        handlerCallBack.callBack(protocolData);
        return null;
    }

    @Override
    public Set<FunctionEnum> interests() {
        return Sets.newHashSet(FunctionEnum.respiratoryArrest);
    }
}
