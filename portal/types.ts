export interface GatewayEvent {
    'body-json': string;
    params: {
        querystring: Record<string, string>,
        header: Record<string, string>
    };
    context: {
        'resource-path': string;
    };
}