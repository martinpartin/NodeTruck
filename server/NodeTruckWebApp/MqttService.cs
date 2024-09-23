using MQTTnet;
using MQTTnet.Client;
using Microsoft.Extensions.Options;

public class MqttService : IMqttService, IDisposable
{
    private readonly IMqttClient _mqttClient;
    private readonly MqttSettings _settings;
    private bool _disposed = false;

    public MqttService(IOptions<MqttSettings> options)
    {
        _settings = options.Value;
        var factory = new MqttFactory();
        _mqttClient = factory.CreateMqttClient();

        var mqttOptions = new MqttClientOptionsBuilder()
            .WithTcpServer(_settings.Broker, _settings.Port)
            .WithCredentials(_settings.Username, _settings.Password)
            .WithCleanSession()
            .Build();

        if (_settings.UseTLS)
        {
            mqttOptions = new MqttClientOptionsBuilder()
                .WithTcpServer(_settings.Broker, _settings.Port)
                .WithCredentials(_settings.Username, _settings.Password)
                .WithTlsOptions( new MqttClientTlsOptions{
                    UseTls = true
                })
                .WithCleanSession()
                .Build();
        }

        _mqttClient.ConnectAsync(mqttOptions).Wait();
    }

    public async Task PublishAsync(string topic, string payload)
    {
        if (!_mqttClient.IsConnected)
        {
            await _mqttClient.ReconnectAsync();
        }

        var message = new MqttApplicationMessageBuilder()
            .WithTopic(topic)
            .WithPayload(payload)
          //  .WithAtLeastOnceQoS()
            .Build();

        await _mqttClient.PublishAsync(message);
    }

    public void Dispose()
    {
        if (!_disposed)
        {
            if (_mqttClient.IsConnected)
            {
                _mqttClient.DisconnectAsync().Wait();
            }
            _mqttClient.Dispose();
            _disposed = true;
        }
    }
}
