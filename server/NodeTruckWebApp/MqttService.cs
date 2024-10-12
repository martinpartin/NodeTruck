using MQTTnet;
using MQTTnet.Client;
using Microsoft.Extensions.Options;
using System.Text;

public class MqttService : IMqttService, IDisposable
{
    private readonly IMqttClient _mqttClient;
    private readonly MqttSettings _settings;
    private bool _disposed = false;

    public event Action<string, string>? OnMessageReceived; 


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


        _mqttClient.ApplicationMessageReceivedAsync += async e =>
        {
            var topic = e.ApplicationMessage.Topic;
            var payload = Encoding.UTF8.GetString(e.ApplicationMessage.PayloadSegment);
            Console.WriteLine($"{topic} {payload}");
            // Trigger eventet når en melding mottas
            OnMessageReceived?.Invoke(topic, payload);

            await Task.CompletedTask; // Påkrevd av signaturen til ApplicationMessageReceivedAsync
        };



    }

    public async Task SubscribeAsync(string topic)
    {
        if (_mqttClient.IsConnected)
        {
            await _mqttClient.SubscribeAsync(new MQTTnet.Client.MqttClientSubscribeOptionsBuilder()
                .WithTopicFilter(topic)
                .Build());
        }
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
