
def prophet(prophecy,start,periods):
    import pandas as pd
    import time
    import datetime
    from fbprophet import Prophet
    from influxdb_client import InfluxDBClient, Point, WriteOptions
    from influxdb_client.client.write_api import SYNCHRONOUS
    import numpy as np
    import requests

    token = '#your_token#'
    bucket = 'myfield'
    org = 'field'
    client = InfluxDBClient(url="#your_ip#:8086", token=token, org=org)
    t = time.localtime()
    next_year = datetime.date.today().year + 1

    # send an api post to clean previous forecast
    ip = "http://#your_ip#:8086/api/v2/delete?org=field&bucket=myfield"
    data = {"start": "2020-10-15T00:00:00Z", "stop": str(next_year)+"-10-15T00:00:00Z", "predicate": 'type="forecast" AND measure="'+str(prophecy)+'"'}
    headers = {'Authorization': 'Token #your_token#=='}
    r = requests.post(url=ip, headers=headers, json=data)

    if(r.status_code<400): print('flush compleate')


    query_api = client.query_api()
    write_api = client.write_api(write_options=SYNCHRONOUS)
    query = 'from(bucket:"test1")' \
            ' |> range(start:' + str(start) + ')'\
            ' |> filter(fn: (r) => r._measurement == "data" and r._field == '+'"' + str(prophecy) + '"' + ')' \
            ' |> filter(fn: (r) =>  r["tag1"] == "1" or r["tag1"] == "2")'

    # try if i can do prediction of both
    #querying data from influx
    result = client.query_api().query(org=org, query=query)
    raw = []

    #here i fillm the raw array with the values obtained by the influx query
    for table in result:
        for record in table.records:
            raw.append((record.get_value(), record.get_time()))


    # print(raw)

    print("=== influxdb query into dataframe ===")
    # here i check if DST is active or not, if DST is active or not

    if t.tm_isdst == 0:
        time_shift = 1
    else:
        time_shift = 2

    # here i shift the whole time frame retrieved from influx of an offset to go from UTC to CET
    df=pd.DataFrame(raw, columns=['y','ds'], index=None)
    df['ds'] = df['ds'] + np.timedelta64(time_shift, 'h')  #keep attention
    df['ds'] = df['ds'].values.astype('<M8[s]')
    df.head()


    m = Prophet(changepoint_prior_scale=0.01)
    m.fit(df)
    #365 specifies the number of time series points you’d like to forecast onto
    future = m.make_future_dataframe(periods=int(periods), freq = 'min', include_history=True)
    #The predict method will assign each row in future a predicted value (yhat). The upper (yhat_upper) and lower (yhat_lower) confidence intervals are also included as a part of the forecast. Columns for components and uncertainty intervals are also included in the forecast, although they aren’t displayed here.
    forecast = m.predict(future)
    forecast[['ds', 'yhat', 'yhat_lower', 'yhat_upper']].tail()


    #here I create the table of forecast data to be uploaded to influx
    forecast['measurement'] = "data"
    cp = forecast[['ds', 'yhat', 'yhat_lower', 'yhat_upper','measurement']].copy()
    lines = [str(cp["measurement"][d])
             + ",type=forecast"
             + ",measure="+str(prophecy)
             + " "
             + str(prophecy)+"_med=" + str(cp["yhat"][d]) + ","
             + str(prophecy)+"_lower=" + str(cp["yhat_lower"][d]) + ","
             + str(prophecy)+"_upper=" + str(cp["yhat_upper"][d])
             + " " + str(int(time.mktime(cp['ds'][d].timetuple()))) + "000000000" for d in range(len(cp))]


    _write_client = client.write_api(write_options=WriteOptions(batch_size=1000,
                                                                flush_interval=10_000,
                                                                jitter_interval=2_000,
                                                                retry_interval=5_000))

    _write_client.write(bucket, org, lines)
    _write_client.__del__()
    client.__del__()
    print('prediction done,uploaded')
